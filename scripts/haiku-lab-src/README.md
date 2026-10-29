# Haiku lab decoder source

The lab page (`haiku-lab.html`, unlinked) compares ways of making a haiku's three lines belong
together. Its decoder is separate from the main generator, which stays as it is:

- `lab_host.cpp`: a copy of `../haiku-src/host.cpp` (the V8.16 decoder since 2026-10-28; V8.10 from 2026-10-25, V8.9 before) with a concept per poem.
  `LAB_STYLE=1` (one-thing) favours the concept's space words in every line; `LAB_STYLE=2`
  (two-part) does so in line one, then switches lines 2-3 to the concept's season and earthly words,
  across the cut. A line with none of its concept words is scored down (`LAB_PICK_BONUS` 5,
  `LAB_MISS` 8, tuned on 200 poems per style on 25 October 2026). `LAB_STYLE=0` writes the same
  poems as the original, checked on 30 space and 30 winter poems.
- `lab_concepts.h`: the ten concepts and their words. `--lab-check` confirms every word is in the
  grammar and allowed in its theme.
- `browser_lab.cpp`: the browser adapter, exporting `haiku_lab_generate(style, concept, case)`.

Headers, `llm.h` and the generated tables are read from `../haiku-src/`, and the model from
`../../haiku-generator/model.bin` (hash-checked). The V8.16 choice on the lab page (page "haiku-lab 5",
2026-10-28) runs `haiku-lab/v8.16/`, the main page's V8.16 generator frozen as it shipped (site-v8.16), so the
lab's ratings stay comparable while the main page moves on. `haiku-lab/v8.10/` is the earlier baseline (page
"haiku-lab 4", 04faca3), kept and tested but no longer offered. `v8.10` ratings received between
2026-10-26 06:51 UTC (V8.11 on the main page) and that pin came from V8.11, not V8.10.

To move the lab to a new release: merge the `../haiku-src/host.cpp` changes into `lab_host.cpp` (a three-way
merge from the previous release's host.cpp), rebuild, freeze the main page's `haiku-generator/` decoder and
the theme cases of `parity.json` into `haiku-lab/v8.N/`, bump `PAGE` in `haiku-lab.html`, and add the variant to
the Apps Script's VARIANTS and the pull script's ROUND.

## Rebuild and test

```sh
python3 scripts/build-haiku-lab.py
node scripts/test-haiku-lab.mjs
```

Native build for host experiments:

```sh
c++ -std=c++17 -O2 -DLLM_INT8_ACT=1 -I scripts/haiku-lab-src -I scripts/haiku-src -I scripts/haiku-src/include \
    scripts/haiku-lab-src/lab_host.cpp -o /tmp/lab
LAB_STYLE=2 /tmp/lab haiku-generator/model.bin grammar 20 space
```

## Ratings

The page sends each rating to the "Haiku Lab Ratings" Google Sheet through its Apps Script
(`haiku-training/scripts/haiku_lab_ratings.gs`; set `RATINGS_URL` in `haiku-lab.html` to the web
app URL). Unsent ratings wait in the browser and are retried. In haiku-training, the task
"Get new haiku lab ratings" (`scripts/pull_lab_ratings.py`) brings them into `data/haiku-lab-ratings/`.
