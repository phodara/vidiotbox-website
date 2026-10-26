// Host port of c5/src/haiku/main.cpp's decoding core, without display or RTOS.
#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <deque>
#include <string>
#include <vector>
#include "llm.h"
#include "generated/vocab.h"
#include "generated/haiku_tables.h"
#include "generated/grammar_tables.h"
#include "haiku_seasons.h"
#include "haiku_grammar_decode.h"
#include "haiku_voice.h"
#include "haiku_settings.h"
#ifdef HAIKU_SCENE_RESEARCH
#include "scene_guidance.h"
#endif

static_assert(HAIKU_VOCAB == VOCAB_N, "tokenizer table mismatch");
static constexpr int TOP_K=HAIKU_TOP_K, TRIES=HAIKU_TRIES, MAX_TOKENS=48, LINE_BYTES=96;
static float WAVE_FALL_PENALTY=HAIKU_WAVE_FALL_PENALTY;   // WAVE_FALL_PENALTY overrides it
static float TEMPERATURE=HAIKU_TEMPERATURE, NATURE_BOOST=HAIKU_NATURE_BOOST, NATURE_SCORE=HAIKU_NATURE_SCORE, FRESHNESS=0.f;
static void read_tuning(const char *name,float *value) {
  const char *text=getenv(name); if (text) *value=(float)atof(text);
}
static const int TARGET[]={5,7,5};
static Model model;
static Scratch s;
static uint32_t used[(G_WORD_CAPACITY+31)/32], rng;
static GWorkspace workspace;
static int season=-1;                 // -1: no theme filtering (original experiment)
static GEntry themed[G_ENTRY_COUNT];
static int themed_count=G_ENTRY_COUNT;
static uint32_t place_seed;           // per-poem subset of place nouns (space only); 0 keeps all
static int place_percent=HAIKU_SPACE_PLACE_PERCENT, verb_percent=HAIKU_SPACE_VERB_PERCENT;
static int adj_percent=HAIKU_SPACE_ADJ_PERCENT, adv_percent=HAIKU_SPACE_ADV_PERCENT,
  person_verb_percent=HAIKU_SPACE_PERSON_VERB_PERCENT;
static float fresh_decay=HAIKU_FRESH_DECAY;
static bool palette_on=true;           // off only while a line retries with the full vocabulary
static long palette_fallbacks;
static bool constrained;
static int pron_class;
static long forwards, rejected, attempts, grammar_filtered;
static char lines[3][LINE_BYTES];
static int line_syl[3];
static int line_history_limit=HAIKU_SPACE_LINE_HISTORY;
static int line_repeat_mode=1;          // 0=off, 1=reject when an alternative exists, 2=penalty.
static float line_repeat_penalty=5.f;
static std::deque<std::string> recent_lines;
static long line_repeat_attempts, line_repeat_fallbacks;
// Earth seasons with the space decoder's variety features (HAIKU_SEASON_FEATURES; SEASON_FEATURES=0 turns off).
static bool season_features=false;
static int season_noun_percent=HAIKU_SEASON_NOUN_PERCENT, season_verb_percent=HAIKU_SEASON_VERB_PERCENT;
static bool featured() { return season==HAIKU_SEASON_SPACE || (season>=0 && season_features); }

static bool is_used(int t) { return used[t>>5] & (1u<<(t&31)); }
static void mark_used(int t) {
  used[t>>5] |= 1u<<(t&31);
  int sibling=haiku_word_sibling(t); if (sibling>=0) used[sibling>>5] |= 1u<<(sibling&31);
  int kin=haiku_word_kin(t);   // neutron also uses up neutrino(s)
  if (kin>=0) { used[kin>>5] |= 1u<<(kin&31); kin=haiku_word_sibling(kin); if (kin>=0) used[kin>>5] |= 1u<<(kin&31); }
}
static float rand01() {
  rng ^= rng << 13; rng ^= rng >> 17; rng ^= rng << 5;
  return rng / 4294967296.0f;
}
static void forward(int token, int pos) {
  if (pos >= model.c.seq_len) { std::fprintf(stderr,"context overflow\n"); std::exit(1); }
  llm_forward(&model,token,pos,&s); forwards++;
}
static float log_norm() {
  float mx=-1e30f;
  for (int v=0;v<model.out_vocab;v++) mx=std::max(mx,s.logits[v]);
  float sum=0;
  for (int v=0;v<model.out_vocab;v++) sum+=expf(s.logits[v]-mx);
  return mx+logf(sum);
}
static const char *tok_text(int token, int *length) {
  *length=VOCAB_OFF[token+1]-VOCAB_OFF[token];
  return reinterpret_cast<const char *>(VOCAB_BLOB+VOCAB_OFF[token]);
}
static void append(char *line, const char *text, int length) {
  int n=(int)strlen(line);
  if (!n) while (length && *text==' ') { text++;length--; }
  length=std::min(length,LINE_BYTES-1-n);
  memcpy(line+n,text,length); line[n+length]=0;
}
static void append_tok(char *line,int token) {
  int length; const char *text=tok_text(token,&length); append(line,text,length);
}
static std::string line_key(const char *line) {
  std::string key(line);
  while (!key.empty() && (key.back()==' ' || key.back()=='.' || key.back()==',')) key.pop_back();
  for (char &ch:key) if (ch>='A' && ch<='Z') ch=(char)(ch-'A'+'a');
  return key;
}
static bool line_was_recent(const char *line) {
  if (!featured() || !line_history_limit || !line_repeat_mode) return false;
  const std::string key=line_key(line);
  for (const std::string &old:recent_lines) if (old==key) return true;
  return false;
}
static void remember_poem_lines() {
  if (!line_history_limit) return;
  for (int i=0;i<3;i++) {
    recent_lines.push_back(line_key(lines[i]));
    while ((int)recent_lines.size()>line_history_limit) recent_lines.pop_front();
  }
}
static void select_theme(int index) {
  season=index; themed_count=0;
  for (const GEntry &entry:G_ENTRIES) {
    GEntry out;
    if (index<0) { themed[themed_count++]=entry; continue; }
    if (entry.token!=UINT16_MAX && (entry.token>=VOCAB_N ? index==HAIKU_SEASON_SPACE || (entry.tag==G_NOUN && (entry.object & G_EARTH_PHRASE)) : haiku_season_allows(entry.token,index)) &&
        haiku_theme_entry(entry,index==HAIKU_SEASON_SPACE,&out)) {
#ifdef HAIKU_SCENE_RESEARCH
      if (index==HAIKU_SEASON_SPACE) out=research_ancient_light(out,research_word(entry.token));
#endif
      themed[themed_count++]=out;
    }
  }
  if (index==HAIKU_SEASON_SPACE) {
    themed_count=haiku_thin_places(themed,themed_count,place_seed,place_percent,verb_percent,haiku_word_token("space"));
    themed_count=haiku_thin_palette(themed,themed_count,place_seed && palette_on ? (place_seed^0x9e3779b9u)|1 : 0,
                                    adj_percent,adv_percent,person_verb_percent);
  } else if (index>=0 && season_features) {
    themed_count=haiku_thin_season(themed,themed_count,place_seed && palette_on ? place_seed : 0,
                                   season_noun_percent,season_verb_percent);
    themed_count=haiku_thin_palette(themed,themed_count,place_seed && palette_on ? (place_seed^0x9e3779b9u)|1 : 0,
                                    adj_percent,adv_percent,100);
  }
}
static const GEntry *grammar_entry(int token) {
  return haiku_grammar_entry(token,themed,themed_count);
}
static const GEntry *grammar_word(const char *word) {
  for (int i=0;i<G_ENTRY_COUNT;i++) {
    if (!strcmp(G_WORDS[i],word)) return G_ENTRIES+i;
  }
  return nullptr;
}
static bool parse_clause(const char *text, GState *state) {
  *state=g_start(); char copy[LINE_BYTES]; snprintf(copy,sizeof copy,"%s",text);
  for (char *p=copy;*p;p++) if (*p>='A' && *p<='Z') *p += 'a'-'A';
  const char *words[32]; int n=0;
  for (char *word=strtok(copy," .,");word && n<32;word=strtok(nullptr," .,")) words[n++]=word;
  // A phrase entry ("in a flash", "waves on the pond") is tried first, then its words one by one.
  for (int i=0;i<n;) {
    int taken=0;
    for (int k=n-i<4 ? n-i : 4;k>=1 && !taken;k--) {
      char word[LINE_BYTES]=""; GState next;
      for (int j=0;j<k;j++) snprintf(word+strlen(word),sizeof word-strlen(word),"%s%s",j ? " " : "",words[i+j]);
      const GEntry *entry=grammar_word(word);
      if (!entry) continue;
#ifdef HAIKU_SCENE_RESEARCH
      GEntry adjusted=season==HAIKU_SEASON_SPACE ? research_ancient_light(*entry,word) : *entry;
      entry=&adjusted;
#endif
      if (g_advance(*state,entry,&next)) { *state=next; taken=k; }
    }
    if (!taken) return false;
    i+=taken;
  }
  return true;
}
static void joins_of(int base,int *lo,int *hi) {
  int a=0,b=HAIKU_N_JOINS;
  while (a<b) { int m=(a+b)/2; if (HAIKU_JOINS[m].base<base) a=m+1; else b=m; }
  *lo=a; while (b<HAIKU_N_JOINS && HAIKU_JOINS[b].base==base) b++; *hi=b;
}
static bool fits(int cost,int rem,bool no_end) { return cost<rem || (cost==rem && !no_end); }
static bool no_start(int token) {
  int a=0,b=HAIKU_N_NO_START;
  while (a<b) { int m=(a+b)/2; if (HAIKU_NO_START[m]<token) a=m+1; else b=m; }
  return a<HAIKU_N_NO_START && HAIKU_NO_START[a]==token;
}
struct Pick { int tok,cost,word; };
struct Shortlist {
  Pick c[TOP_K]; float x[TOP_K]; int k=0;
  void offer(int tok,int cost,float rank,int word) {
    if (k==TOP_K && rank<=x[k-1]) return;
    int j=k<TOP_K ? k++ : k-1;
    while (j>0 && x[j-1]<rank) { x[j]=x[j-1]; c[j]=c[j-1]; j--; }
    x[j]=rank; c[j]={tok,cost,word};
  }
};
static bool grammar_allows(int word,int cost,int rem,GState state) {
  if (!constrained) return true;
  const GEntry *e=grammar_entry(word); GState next;
  bool ok=e && e->syl==cost && g_advance(state,e,&next) &&
    g_can_finish(&workspace,next,rem-cost,themed,themed_count);
  if (!ok) grammar_filtered++;
  return ok;
}
static float scene_bonus(int word,GState state) {
#ifdef HAIKU_SCENE_RESEARCH
  const GEntry *entry=grammar_entry(word);
  if (constrained && season==HAIKU_SEASON_SPACE && entry) return research_scene_bonus(*entry,state);
#else
  (void)word; (void)state;
#endif
  return 0.f;
}
static Pick pick(int rem,int prev,bool sentence_start,GState state,int budget) {
  Shortlist sl; int lo=0,hi=0;
  if (prev>=0) joins_of(prev,&lo,&hi);
  bool prev_frag=!constrained && prev>=0 && (HAIKU_TOK[prev]&HAIKU_FRAG);
  int prev_syl=prev>=0 ? HAIKU_TOK[prev]&HAIKU_SYL_MASK : 0;
  if (!constrained) for (int i=lo;i<hi;i++) {
    const HaikuJoin &j=HAIKU_JOINS[i]; int cost=std::max(0,j.syl-prev_syl);
    if (fits(cost,rem,j.flags&HAIKU_NO_END)) sl.offer(j.suffix,cost,s.logits[j.suffix],-1);
  }
  if (!constrained && !prev_frag) for (int v=0;v<VOCAB_N;v++) {
    uint8_t f=HAIKU_TOK[v];
    if (!f || is_used(v) || (sentence_start && no_start(v))) continue;
    if (season>=0 && !haiku_season_allows(v,season)) continue;
    if (f&HAIKU_FRAG) {
      if (constrained) continue;
      int a,b; joins_of(v,&a,&b); bool ok=false;
      for (int i=a;i<b && !ok;i++) ok=fits(HAIKU_JOINS[i].syl,rem,HAIKU_JOINS[i].flags&HAIKU_NO_END);
      if (ok) sl.offer(v,0,s.logits[v],v);
      continue;
    }
    int n=f&HAIKU_SYL_MASK, pc=HAIKU_PRON(f);
    if (!fits(n,rem,f&HAIKU_NO_END) || (pc && pc!=pron_class)) continue;
    if (!grammar_allows(v,n,rem,state)) continue;
    sl.offer(v,n,s.logits[v]+((f&HAIKU_NATURE)?NATURE_BOOST:0),v);
  }
  if (sentence_start) for (int i=0;i<HAIKU_N_CAPS;i++) {
    int word=HAIKU_CAPS[i].word; uint8_t f=HAIKU_TOK[word];
    int n=f&HAIKU_SYL_MASK,pc=HAIKU_PRON(f);
    if (is_used(word) || !fits(n,rem,f&HAIKU_NO_END) || (pc && pc!=pron_class)) continue;
    if (season>=0 && !haiku_season_allows(word,season)) continue;
    if (!grammar_allows(word,n,rem,state)) continue;
    sl.offer(HAIKU_CAPS[i].cap,n,s.logits[HAIKU_CAPS[i].cap]+scene_bonus(word,state),word);
  }
  if (constrained) for (int i=0;i<themed_count;i++) {
    const GEntry &e=themed[i];
    if (e.token==UINT16_MAX || is_used(e.token) || haiku_moonless_crater(e.token,used) ||
        haiku_word_length(e.token)>budget || !grammar_allows(e.token,e.syl,rem,state)) continue;
    int first=haiku_word_piece(e.token,0);
    // First-piece shortlist ranking; every forced continuation is scored below.
    sl.offer(first,e.syl,s.logits[first]-haiku_freshness_penalty(e.token,FRESHNESS)+haiku_space_vocabulary_bonus(e,season==HAIKU_SEASON_SPACE)-haiku_wave_fall_penalty(e,state,WAVE_FALL_PENALTY)+(e.token<VOCAB_N && (HAIKU_TOK[e.token]&HAIKU_NATURE)?NATURE_BOOST:0)+scene_bonus(e.token,state),e.token);
  }
  if (!sl.k) return {-1,0,-1};
  float probabilities[TOP_K],sum=0;
  for (int i=0;i<sl.k;i++) { probabilities[i]=expf((sl.x[i]-sl.x[0])/TEMPERATURE); sum+=probabilities[i]; }
  float r=rand01()*sum;
  for (int i=0;i<sl.k;i++) { r-=probabilities[i]; if (r<=0) return sl.c[i]; }
  return sl.c[sl.k-1];
}
struct Attempt { int ids[MAX_TOKENS],cost[MAX_TOKENS],word[MAX_TOKENS],n; float score; };
static bool attempt_line(int line,int syl,int pos,GState grammar,Attempt *a) {
  a->n=0; float tok_lp=0,fresh=0,scene_score=0; int nature=0,prev=-1;
  while (a->n<MAX_TOKENS) {
    bool open=!constrained && prev>=0 && (HAIKU_TOK[prev]&HAIKU_FRAG);
    if (syl>=TARGET[line] && !open) break;
    Pick pk=pick(TARGET[line]-syl,prev,line==1 && a->n==0,grammar,std::min(MAX_TOKENS-a->n,model.c.seq_len-pos-3));
    if (pk.tok<0) break;
    scene_score+=scene_bonus(pk.word,grammar);
    if (constrained) {
      GState next;
      if (!g_advance(grammar,grammar_entry(pk.word),&next)) return false;
      grammar=next;
    }
    int pieces=constrained ? haiku_word_length(pk.word) : 1;
    if (pieces<1 || a->n+pieces>MAX_TOKENS || pos+pieces>=model.c.seq_len-2) return false;
    if (pk.word>=0) mark_used(pk.word);
    if (pk.word>=0 && pk.word<VOCAB_N && (HAIKU_TOK[pk.word]&HAIKU_NATURE)) nature++;
    if (constrained && pk.word>=0) fresh+=haiku_freshness_penalty(pk.word,FRESHNESS);
    for (int piece=0;piece<pieces;piece++) {
      int token=piece==0 ? pk.tok : haiku_word_piece(pk.word,piece);
      tok_lp+=s.logits[token]-log_norm();
      a->ids[a->n]=token; a->word[a->n]=piece==0?pk.word:-1;
      a->cost[a->n++]=piece==0?pk.cost:0;
      forward(token,pos++);
      prev=token;
    }
    syl+=pk.cost;
  }
  // Line one may be a seed that is already a whole line (n==0).
  if (syl!=TARGET[line] || (!a->n && line!=0) || (!constrained && prev>=0 && (HAIKU_TOK[prev]&HAIKU_FRAG)) ||
      (constrained && !g_complete(grammar))) return false;
  float end=std::max(s.logits[HAIKU_TOK_COMMA],std::max(s.logits[HAIKU_TOK_PERIOD],s.logits[HAIKU_TOK_ELLIPSIS]));
  // Freshness lowers stale words' log-probability, so best-of-five does not undo it.
  a->score=(a->n?(tok_lp-fresh+scene_score)/a->n:0.f)+HAIKU_END_WEIGHT*(end-log_norm())+NATURE_SCORE*nature;
  return true;
}
static bool free_line_one=false;   // the model writes line one itself (HAIKU_FREE_LINE_ONE, or the space mix)
static bool person_voice=false, person_end=false;
static int person_end_percent=HAIKU_SPACE_PERSON_END_PERCENT;
static int canned_percent=HAIKU_SPACE_CANNED_PERCENT, person_percent=HAIKU_SPACE_PERSON_PERCENT;
static int free_percent=0;         // Legacy host override; canned applies only to non-person poems.
static int write_line(int line,const char *prefix,int pos) {
#ifdef HAIKU_SCENE_RESEARCH
  research_line=line;
#endif
  // Explicit five-syllable nominal opening; later lines still require clauses.
  if (line==0 && line_syl[0]==5 && haiku_nominal_seed(prefix)) {
    snprintf(lines[0],LINE_BYTES,"%s",prefix);
    return pos;
  }
  uint32_t used_before[sizeof used/sizeof used[0]]; memcpy(used_before,used,sizeof used);
  std::vector<float> logits_before(s.logits,s.logits+model.out_vocab);
  GState grammar=g_start();
  if (constrained && !parse_clause(prefix,&grammar)) {
    std::fprintf(stderr,"unsupported seed: %s\n",prefix); std::exit(1);
  }
  // Same rule as the firmware: space keeps is/are out of line two, allows it in line three.
  if (constrained && featured() && (line==1 || line==2 || (line==0 && free_line_one))) {
    select_theme(season);
    // A framed line three is a person doing something: no "my eyes are cold".
    // A plain seasonal line three uses is/are in half the poems (review round 7).
    if (line==1 || line==0 || (line==2 && (person_end || (season!=HAIKU_SEASON_SPACE &&
        !haiku_keeps_copula(place_seed,HAIKU_SEASON_COPULA_PERCENT))))) themed_count=haiku_drop_copulas(themed,themed_count);
    memset(&workspace,0,sizeof workspace);
  }
  if (line==0 && free_line_one && featured()) grammar=haiku_voice_start(person_voice);
  if (line==2 && person_end) grammar=haiku_voice_start_as(haiku_voice_of(lines[0]));   // same voice as line one
  Attempt best={},best_repeat={},cur; best.score=best_repeat.score=-1e30f;
  auto tries=[&]() {
    for (int t=0;t<TRIES;t++) {
      memcpy(used,used_before,sizeof used); memcpy(s.logits,logits_before.data(),model.out_vocab*sizeof(float));
      attempts++;
      if (!attempt_line(line,line_syl[line],pos,grammar,&cur)) { rejected++; continue; }
      char candidate[LINE_BYTES]; snprintf(candidate,sizeof candidate,"%s",prefix);
      for (int i=0;i<cur.n;i++) append_tok(candidate,cur.ids[i]);
      const bool repeated=line_was_recent(candidate);
      if (repeated) {
        line_repeat_attempts++;
        if (line_repeat_mode==2) cur.score-=line_repeat_penalty;
        if (cur.score>best_repeat.score) best_repeat=cur;
        if (line_repeat_mode==1) continue;
      }
      if (cur.score>best.score) best=cur;
    }
  };
  tries();
  // The poem's palette can leave a line no way to finish; retry it with every word.
  if (!best.n && constrained && featured()) {
    palette_on=false; palette_fallbacks++;
    if (line==2 && person_end) grammar=haiku_voice_start(false);   // drop the framing voice: no person at all
    select_theme(season);
    if (line==1 || (line==0 && free_line_one)) themed_count=haiku_drop_copulas(themed,themed_count);
    memset(&workspace,0,sizeof workspace);
    tries();
    palette_on=true;
    select_theme(season);
    memset(&workspace,0,sizeof workspace);
  }
  if (!best.n && best_repeat.n) { best=best_repeat; line_repeat_fallbacks++; }
  memcpy(used,used_before,sizeof used); memcpy(s.logits,logits_before.data(),model.out_vocab*sizeof(float));
  snprintf(lines[line],LINE_BYTES,"%s",prefix);
  for (int i=0;i<best.n;i++) {
    int token=best.ids[i]; if (best.word[i]>=0) { mark_used(best.word[i]); if (constrained) haiku_note_word(best.word[i]); }
    line_syl[line]+=best.cost[i]; append_tok(lines[line],token); forward(token,pos++);
  }
  return pos;
}
static void json_string(const char *text) {
  putchar('"');
  for (;*text;text++) {
    if (*text=='"' || *text=='\\') putchar('\\');
    if (*text=='\n') printf("\\n"); else putchar(*text);
  }
  putchar('"');
}
static bool grammar_test(const char *text,int expected) {
  GState state; bool actual=parse_clause(text,&state) && g_complete(state);
  if (actual!=!!expected) { fprintf(stderr,"grammar test failed: %s\n",text); return false; }
  return true;
}
static int self_test() {
  bool ok=true;
  for (const char *text : {"Birds chirp", "A duck swims", "Ducks swim in the pond", "Frogs swim in a lake", "Sunlight shines through the trees", "Frogs sing by a rock", "Light fades under a stone", "Wet leaves glisten", "Crows fly in the rain", "Petals float on the pond", "Leaves float over the field", "A star fades through the leaves", "Mist rises", "A full moon glows", "Twigs float over the old field", "Frogs sing in the pond", "Bees hum in the field", "Light glows on a soft branch", "Light glows through the leaves", "A worm crawls through the grass", "A beetle crawls on the stone", "Wind blows over the pond", "Waves on the lake ripple", "A wave on the pond sparkles", "Waves on the brook sparkle", "Wind stirs the waves on the pond", "Snow melts", "Wet snow melts", "Damp fog drifts", "A wave on the stream shines", "Crows fly over the waves on the lake", "Wind blows through the rain", "Petals float on the pond", "Leaves fall on the grass", "Birds sing in the trees", "Rain falls in the field", "A star fades in fog", "Sunlight shines through the path", "Birds sing on the shore", "Frogs sing in the field", "Crows fly over the lake", "A moon shines over the lake", "Sunlight shines on a dark lake", "Cold rain falls on leaves", "Rain falls in the pond", "Sunlight shines through the trees", "The moon glows through the fog", "The stream flows by the pines", "The pond sparkles", "Spring rain falls", "Winter wind blows", "Crows fly through the rain", "Birds fly in the fog", "Birds perch on the fence", "Petals fall on the grass", "Snow falls on the pond", "Light shines on the lake", "Bees hum", "Crickets jump", "Gulls perch", "Frogs sing", "I breathe quietly", "We gasp", "I blink", "My palms open", "My hand opens", "I blink at stars", "We blink at the moon", "Now I blink at stars", "Stars blink over moons", "Full moons glow", "Birds sing in the reeds", "Crows perch in the old oak", "Gulls perch on the fence", "The tree holds rain", "Bees hum in the flowers", "The lake sparkles by the pines", "An atom whispers", "The wind whispers softly", "Dark matter glows"}) ok &= grammar_test(text,1);
  for (const char *text : {"Stars breathe", "An atom is dark", "Planets are dark", "Rocks whisper", "The atoms whisper in space", "My mind breathes", "My palms opens", "My heart opens", "I breathe past moons", "I blink past moons", "I open", "Palms open", "I blink by stars", "I blink around stars", "I now blink at stars", "We now wait", "Stars blink at moons", "I wait at stars", "I tremble at moons", "Ducks chirp", "A duck chirps", "Bees sing", "Worms fly", "Worms dart", "Frogs fly", "Crows swim", "Bees chirp", "Ducks swim in the fence", "Ducks swim on the path", "Frogs swim in a pine", "The sun rises in a rock", "Light shines through a stone", "Crows fly in the fence", "Leaves drift through sand", "Stones shine", "Water is dry", "A star fades on fog", "Leaves fall on the mist", "Snow falls upon the wind", "Sunlight shines in the path", "Frogs sing in the shore", "Petals fall on bees", "Rain falls on birds", "Frogs sing on the field", "Bees hum upon a meadow", "Crows fly on the lake", "A bird flies upon the pond", "Crows fly in the pond", "Bees fly through the water", "The water flows through a leaf", "A lake falls", "Puddles drift through the garden", "A moon shines in waves", "The sun rises in a field", "Sunlight shines through a dark lake", "The moon glows through a puddle", "Cold rain falls in leaves", "Snow falls in the pines", "Wet stones glisten", "The pale light glows through a root", "Ducks fly in the dry garden", "Birds float through rain", "Crows float by a stone", "A sun fades through mud", "A star fades through streams", "Tall grass sways on leaves", "A pond rises", "Light is full by rocks", "Twigs float under the old field", "Leaves float beneath the pond", "Bees hum in the lake", "Crickets sing in a puddle", "Light glows in a soft branch", "The moon shines in the trees", "A worm crawls through the pond", "Beetles crawl in the water", "The old shore sparkles", "Wind rises through waves", "Wind blows in a puddle", "A tree falls", "Branches drift", "Tall grass sways in leaves", "Leaves float on cold stones", "Birds float on ice", "An old sand is wet", "A star fades on leaves", "Green leaves are winter", "Full light glows", "Full stars warm wet fog", "A full sun burns", "The bird sings in a soft leaf", "Birds sing on the stone", "Crows perch upon a rock", "A tree holds cold water", "An old pine holds streams", "The lake sparkles on a pond", "The stream flows in the lake", "Bees hum on the lake", "Some bees rest on the water"}) ok &= grammar_test(text,0);
  for (const char *text : {"Rain falls softly", "Birds sing in the trees", "I tremble by moons", "We watch the comet", "I wait by asteroids here", "Comets glow by moons there", "I wander past the moons", "I wonder softly", "We watch the cosmos", "The sun pulls light", "The wind stirs leaves",
                          "A cold pond freezes", "An old tree sways", "Snow is cold", "Mass bends light", "Now time and space shift apart", "Now stars drift", "Light now burns in space", "Time and space now shift apart", "Stars now drift softly", "Space and time are one", "Planets orbit stars", "Time in a flash shifts", "A quasar in a flash fades", "A star in a flash vanishes", "Density shifts",
                          "Signals fade", "The plasma flows", "A pulsar spins", "Light waves fade", "Light waves bend", "Starlight curves past the sun", "Radio waves bend around stars", "A wave of light fades", "Radio waves fade", "Gravitational waves ripple", "Waves of sound fade", "A nebula glows", "Stars are like music", "Light is like a song", "Hydrogen glows", "The wavelength grows", "Neutrinos drift", "Stellar light fades"}) ok &= grammar_test(text,1);
  for (const char *text : {"Bird sing", "Birds sings", "Waves fade", "A wave fades", "Waves ripple", "Waves on the pond fly", "Waves on the brook melt", "Roots hold a wave on the stream", "The wet waves on the lake ripple", "A damp pond sparkles", "Light waves bend light", "Mass bends", "Light waves bend slowly", "Gravitational waves bend", "Light shines on the waves", "A light wave of light fades", "The rain fall", "Rain falls in pond",
                          "The wind stirs", "The wind stirs the", "An tree sways", "A old tree sways",
                          "The snow is", "Rain falls the", "Trees grows", "Leaves", "It falls", "Signals orbit stars", "A crater transmits signals",
                          "A rocket sings", "One time and space shift", "Light burns in space now", "Stars drift now", "Light now burns now", "Time now and space shift", "Now now stars drift", "Stars drift now now", "Now the now stars drift", "The one moon glows", "Now", "The crew watches stars", "The clock floats",
                          "The probe scans", "My eyes warm a wave", "We wait in dust", "I wait past a moon", "Planets are bright there", "We wonder around stars", "Planets orbit a cosmos", "Surfaces are still", "I wait in soft waves", "Energy is cool", "The old sun pulls a cold wave", "Planets orbits stars", "Music floats", "Sounds fade", "Stars drift through music", "Light is music", "A lander glows", "Hydrogen orbits stars", "The wavelength glows", "A neutrinos drift", "Density and a flash shift", "A flash and density shift", "A void in a flash is cold", "Density in a flash now shifts", "Density now in a flash shifts", "Density shifts in a flash", "In a flash density shifts", "Mass in a flash bends light", "I in a flash blink", "An atom in a flash drifts", "Halos in a flash float far", "Halos in a flash pulse fast"}) ok &= grammar_test(text,0);
  for (int i=0;i<G_ENTRY_COUNT;i++) {
    int word=G_ENTRIES[i].token;
    if (word==UINT16_MAX) continue;
    int n=haiku_word_length(word);
    ok &= n>=1 && n<=G_MAX_WORD_TOKENS;
    for (int j=0;j<n;j++) ok &= haiku_word_piece(word,j)>=0 && haiku_word_piece(word,j)<VOCAB_N;
  }
  // A split-token noun must be available as one grammatical/meter choice,
  // fit the token budget, and share one no-repeat identity across all pieces.
  const GEntry *nebula=grammar_word("nebula"), *glows=grammar_word("glows");
  if (!nebula || !glows || nebula->token<VOCAB_N) return 1;
  themed[0]=*glows; themed[1]=*nebula; themed_count=2;
  if (themed[0].token>themed[1].token) std::swap(themed[0],themed[1]);
  std::vector<float> test_logits(VOCAB_N,-100.f);
  s.logits=test_logits.data(); constrained=true; rng=123; season=HAIKU_SEASON_SPACE;
  memset(used,0,sizeof used); memset(&workspace,0,sizeof workspace);
  GState noun_state;
  ok &= parse_clause("A",&noun_state);
  int length=haiku_word_length(nebula->token);
  Pick selected=pick(4,-1,false,noun_state,length);
  ok &= selected.word==nebula->token && selected.cost==3;
  ok &= pick(4,-1,false,noun_state,length-1).tok<0;
  mark_used(nebula->token);
  ok &= pick(4,-1,false,noun_state,length).tok<0;
  char reconstructed[LINE_BYTES]={0};
  for (int piece=0;piece<length;piece++) append_tok(reconstructed,haiku_word_piece(nebula->token,piece));
  ok &= !strcmp(reconstructed,"nebula");
  memset(used,0,sizeof used); constrained=false; s.logits=nullptr;
  select_theme(-1);
  GState high=g_start(), low=g_start(); high.subject=G_CRAFT; low.subject=G_AIR;
  ok &= g_key(high,1)!=g_key(low,1);
  GState state=g_start();
  if (!parse_clause("The wind stirs",&state)) return 1;
  memset(&workspace,0,sizeof workspace);
  ok &= !g_can_finish(&workspace,state,0,G_ENTRIES,G_ENTRY_COUNT);
  ok &= g_can_finish(&workspace,state,1,G_ENTRIES,G_ENTRY_COUNT);
  for (const HaikuSeed &seed:HAIKU_SEEDS) {
    if (haiku_nominal_seed(seed.text)) { ok &= seed.syl==5; continue; }
    select_theme(seed.season);
    // A seed may end mid-phrase ("Stars drift past") if a legal completion fits line one.
    bool parsed=parse_clause(seed.text,&state);
    memset(&workspace,0,sizeof workspace);
    bool feasible=g_can_finish(&workspace,state,5-seed.syl,themed,themed_count);
    if (!parsed || !feasible) fprintf(stderr,"seed failed: %s parsed=%d feasible=%d\n",seed.text,parsed,feasible);
    ok &= parsed && feasible;
  }
  select_theme(-1);
  printf("{\"self_test\":%s,\"grammar_workspace_bytes\":%zu,\"grammar_table_bytes\":%zu,\"grammar_state_bytes\":%zu}\n",
         ok?"true":"false",sizeof workspace,sizeof G_ENTRIES,sizeof(GState));
  return ok?0:1;
}
static float *allocate(size_t count) {
  float *p=(float *)calloc(count,sizeof(float));
  if (!p) { fprintf(stderr,"allocation failed\n"); exit(1); } return p;
}
int main(int argc,char **argv) {
  if (argc==2 && !strcmp(argv[1],"--self-test")) return self_test();
  if (argc!=4 && argc!=5) { fprintf(stderr,"usage: decoder model.bin baseline|grammar count|interactive [theme]\n"); return 2; }
  int chosen=-1;
  if (argc==5) {
    static const char *const names[]={"spring","summer","autumn","winter","space"};
    for (int i=0;i<HAIKU_N_SEASONS;i++) if (!strcmp(argv[4],names[i])) chosen=i;
    if (chosen<0) { fprintf(stderr,"unknown theme %s\n",argv[4]); return 2; }
  }
  if (chosen==HAIKU_SEASON_SPACE) {
    // Same as the firmware's HAIKU_SPACE_* values; keep them in step.
    TEMPERATURE=HAIKU_SPACE_TEMPERATURE; NATURE_BOOST=HAIKU_SPACE_NATURE_BOOST; NATURE_SCORE=HAIKU_SPACE_NATURE_SCORE; FRESHNESS=HAIKU_SPACE_FRESHNESS;
    read_tuning("SPACE_TEMPERATURE",&TEMPERATURE); read_tuning("SPACE_NATURE_BOOST",&NATURE_BOOST);
    read_tuning("SPACE_NATURE_SCORE",&NATURE_SCORE); read_tuning("SPACE_FRESHNESS",&FRESHNESS);
    float percent=(float)place_percent, verbs=(float)verb_percent;
    read_tuning("SPACE_PLACE_PERCENT",&percent); read_tuning("SPACE_VERB_PERCENT",&verbs);
    place_percent=(int)percent; verb_percent=(int)verbs;
    auto percent_setting=[](const char *name,int fallback) {
      const char *value=getenv(name); if (!value) return fallback;
      char *end=nullptr; long number=strtol(value,&end,10);
      if (!*value || *end || number<0 || number>100) { fprintf(stderr,"invalid %s\n",name); exit(2); }
      return (int)number;
    };
    canned_percent=percent_setting("SPACE_CANNED_PERCENT",HAIKU_SPACE_CANNED_PERCENT);
    person_percent=percent_setting("SPACE_PERSON_PERCENT",HAIKU_SPACE_PERSON_PERCENT);
    adj_percent=percent_setting("SPACE_ADJ_PERCENT",adj_percent);
    adv_percent=percent_setting("SPACE_ADV_PERCENT",adv_percent);
    person_verb_percent=percent_setting("SPACE_PERSON_VERB_PERCENT",person_verb_percent);
    person_end_percent=percent_setting("SPACE_PERSON_END_PERCENT",person_end_percent);
    read_tuning("SPACE_FRESH_DECAY",&fresh_decay);
    if (!(fresh_decay>=0.f && fresh_decay<1.f)) { fprintf(stderr,"invalid SPACE_FRESH_DECAY\n"); return 2; }
    free_percent=getenv("SPACE_FREE_PERCENT") ? percent_setting("SPACE_FREE_PERCENT",0) : 100-canned_percent;
    if (const char *value=getenv("SPACE_LINE_HISTORY")) {
      char *end=nullptr; long number=strtol(value,&end,10);
      if (!*value || *end || number<0 || number>100000) { fprintf(stderr,"invalid SPACE_LINE_HISTORY\n"); return 2; }
      line_history_limit=(int)number;
    }
    if (const char *value=getenv("SPACE_LINE_REPEAT_MODE")) {
      if (!strcmp(value,"reject")) line_repeat_mode=1;
      else if (!strcmp(value,"penalty")) line_repeat_mode=2;
      else if (!strcmp(value,"off")) line_repeat_mode=0;
      else { fprintf(stderr,"invalid SPACE_LINE_REPEAT_MODE\n"); return 2; }
    }
    read_tuning("SPACE_LINE_REPEAT_PENALTY",&line_repeat_penalty);
    if (!(line_repeat_penalty>=0.f) || !std::isfinite(line_repeat_penalty)) { fprintf(stderr,"invalid SPACE_LINE_REPEAT_PENALTY\n"); return 2; }
  }
  read_tuning("WAVE_FALL_PENALTY",&WAVE_FALL_PENALTY);
  if (!(WAVE_FALL_PENALTY>=0.f) || !std::isfinite(WAVE_FALL_PENALTY)) { fprintf(stderr,"invalid WAVE_FALL_PENALTY\n"); return 2; }
  const char *season_switch=getenv("SEASON_FEATURES");   // default: the firmware's HAIKU_SEASON_FEATURES
  if (chosen>=0 && chosen!=HAIKU_SEASON_SPACE && (season_switch ? strcmp(season_switch,"0")!=0 : HAIKU_SEASON_FEATURES)) {
    // Same as the firmware's HAIKU_SEASON_* values; keep them in step.
    season_features=true;
    FRESHNESS=HAIKU_SEASON_FRESHNESS; read_tuning("SEASON_FRESHNESS",&FRESHNESS);
    auto percent_setting=[](const char *name,int fallback) {
      const char *value=getenv(name); if (!value) return fallback;
      char *end=nullptr; long number=strtol(value,&end,10);
      if (!*value || *end || number<0 || number>100) { fprintf(stderr,"invalid %s\n",name); exit(2); }
      return (int)number;
    };
    season_noun_percent=percent_setting("SEASON_NOUN_PERCENT",season_noun_percent);
    season_verb_percent=percent_setting("SEASON_VERB_PERCENT",season_verb_percent);
    adj_percent=percent_setting("SEASON_ADJ_PERCENT",HAIKU_SEASON_ADJ_PERCENT);
    adv_percent=percent_setting("SEASON_ADV_PERCENT",HAIKU_SEASON_ADV_PERCENT);
    free_percent=percent_setting("SEASON_FREE_PERCENT",HAIKU_SEASON_FREE_PERCENT);
  }
  select_theme(chosen);
#ifdef HAIKU_SCENE_RESEARCH
  if (chosen==HAIKU_SEASON_SPACE) {
    research_scene_weight=0.f;
    read_tuning("SPACE_SCENE_WEIGHT",&research_scene_weight);
    if (!std::isfinite(research_scene_weight) || research_scene_weight<0.f || research_scene_weight>10.f) {
      fprintf(stderr,"invalid SPACE_SCENE_WEIGHT\n"); return 2;
    }
  }
#endif
  if (strcmp(argv[2],"baseline") && strcmp(argv[2],"grammar")) return 2;
  // Simulator protocol: one case number per stdin line, one JSON poem per stdout line.
  // Keep the model and recent-word history alive between requests.
  bool interactive=!strcmp(argv[3],"interactive");
  constrained=!strcmp(argv[2],"grammar"); int cases=interactive ? 1000 : atoi(argv[3]);
  if (cases<1 || cases>1000) return 2;
  FILE *file=fopen(argv[1],"rb"); if (!file) { perror(argv[1]); return 1; }
  fseek(file,0,SEEK_END); long bytes=ftell(file); rewind(file);
  if (bytes<LLM_HEADER_BYTES) return 1;
  std::vector<uint8_t> blob(bytes);
  if (fread(blob.data(),1,bytes,file)!=(size_t)bytes) return 1;
  fclose(file);
  if (llm_load(blob.data(),&model) || model.image_bytes!=blob.size() || model.out_vocab!=VOCAB_N) return 1;
  int D=model.c.dim,L=model.c.n_layers,P=model.c.ple_dim,F=model.c.ffn,V=model.out_vocab,S=model.c.seq_len;
  size_t scratch_count=D+std::max(F,D)+3*D+D+F+std::max(P,F)+3*L*P+V+S+2*L*S*D;
  s.x=allocate(D); s.h=allocate(std::max(F,D)); s.qkv=allocate(3*D); s.att=allocate(D);
  s.g1=allocate(F); s.g2=allocate(std::max(P,F)); s.ple=allocate(L*P);
  s.tmpP=allocate(L*P); s.trow=allocate(L*P); s.logits=allocate(V); s.scores=allocate(S);
  s.kcache=allocate((size_t)L*S*D); s.vcache=allocate((size_t)L*S*D);
  int start_case=0;
  if (const char *text=getenv("HAIKU_START_CASE")) {
    char *end=nullptr; long value=strtol(text,&end,10);
    if (!*text || *end || value<0 || value>1000000000) {
      fprintf(stderr,"invalid HAIKU_START_CASE\n"); return 2;
    }
    start_case=(int)value;
  }
  for (int offset=0;interactive || offset<cases;offset++) {
    int c=start_case+offset;
    if (interactive && (scanf("%d",&c)!=1 || c<0 || c>1000000000)) break;
#ifdef HAIKU_SCENE_RESEARCH
    research_scene=chosen==HAIKU_SEASON_SPACE && research_scene_weight>0.f ? c%4 : -1;
#endif
    const HaikuSeed *pool[HAIKU_N_SEEDS]; int pool_n=0;
    for (const HaikuSeed &candidate:HAIKU_SEEDS)
      if ((chosen<0 || candidate.season==chosen) && (chosen!=HAIKU_SEASON_SPACE || free_percent==0 || candidate.syl==5)) pool[pool_n++]=&candidate;
    const HaikuSeed &seed=*pool[c%pool_n];
    place_seed=((uint32_t)(6001+c)*2654435761u)|1;
    select_theme(chosen);
    rng=6001+c; uint32_t initial_seed=rng; pron_class=seed.pron;
    memset(lines,0,sizeof lines); memset(line_syl,0,sizeof line_syl); memset(used,0,sizeof used);
    haiku_decay_recent(fresh_decay);   // freshness carries across poems in one run
    memset(&workspace,0,sizeof workspace); forwards=rejected=attempts=grammar_filtered=palette_fallbacks=0;
    line_repeat_attempts=line_repeat_fallbacks=0;
    auto started=std::chrono::steady_clock::now(); int pos=0;
    // Experiment: HAIKU_FREE_LINE_ONE=<start token id> lets the model write line one itself,
    // starting from that token (the end-of-text token that precedes each training poem).
    const char *free_start=getenv("HAIKU_FREE_LINE_ONE");
    if (!free_start && (chosen==HAIKU_SEASON_SPACE || season_features) && free_percent>0 &&
        (int)(((uint32_t)(c+1)*2246822519u)>>16)%100<free_percent) free_start="0";
    free_line_one=free_start!=nullptr;
    // Independent deterministic draws keep voice separate from opener count.
    person_voice=constrained && chosen==HAIKU_SEASON_SPACE &&
      (int)(((uint32_t)(c+1)*3266489917u)>>16)%100<person_percent;
    // Person poems always generate their first line; canned lines have no guaranteed voice.
    if (person_voice) { free_start="0"; free_line_one=true; }
    person_end=person_voice && (int)(((uint32_t)(c+1)*2654435761u)>>17)%100<person_end_percent;
    const char *first_prefix=free_start?"":seed.text;
    if (free_start) forward(atoi(free_start),pos++);
    else {
      for (int i=0;i<seed.n;i++) { mark_used(seed.ids[i]); forward(seed.ids[i],pos++); }
      if (constrained) haiku_mark_prefix_words(seed.text,mark_used);
      line_syl[0]=seed.syl;
    }
    for (int line=0;line<3;line++) {
      pos=write_line(line,line==0?first_prefix:"",pos);
      if (constrained) haiku_capitalize_i(lines[line]);
      if (line==0 && free_line_one && lines[0][0]>='a' && lines[0][0]<='z') lines[0][0]-='a'-'A';
      if (line==0) { append(lines[0],"...",3); forward(HAIKU_TOK_PERIOD,pos++); }
      if (line==1) forward(HAIKU_TOK_COMMA,pos++);
      if (line==2) append(lines[2],".",1);
    }
    remember_poem_lines();
    double seconds=std::chrono::duration<double>(std::chrono::steady_clock::now()-started).count();
    printf("{\"case\":%d,\"seed\":%u,\"mode\":\"%s\",\"prompt\":",c,initial_seed,argv[2]); json_string(first_prefix);
    printf(",\"person_voice\":%s,\"person_end\":%s,\"generated_first_line\":%s,\"lines\":[",person_voice?"true":"false",person_end?"true":"false",free_line_one?"true":"false");
    for (int i=0;i<3;i++) { if (i) putchar(','); json_string(lines[i]); }
    printf("],\"syllables\":[%d,%d,%d],\"complete\":%s,\"attempts\":%ld,\"rejected_attempts\":%ld,\"forwards\":%ld,\"grammar_filtered\":%ld,\"palette_fallbacks\":%ld,\"line_repeat_attempts\":%ld,\"line_repeat_fallbacks\":%ld,\"host_seconds\":%.6f,\"scratch_bytes\":%zu,\"saved_logits_bytes\":%zu,\"used_bits_bytes\":%zu,\"grammar_workspace_bytes\":%zu,\"grammar_table_bytes\":%zu,\"grammar_state_bytes\":%zu}\n",
           line_syl[0],line_syl[1],line_syl[2],line_syl[0]==5 && line_syl[1]==7 && line_syl[2]==5?"true":"false",
           attempts,rejected,forwards,grammar_filtered,palette_fallbacks,line_repeat_attempts,line_repeat_fallbacks,seconds,scratch_count*sizeof(float),V*sizeof(float),sizeof used,
           constrained?sizeof workspace:0,constrained?sizeof G_ENTRIES:0,constrained?sizeof(GState):0);
    fflush(stdout);
  }
  return 0;
}
