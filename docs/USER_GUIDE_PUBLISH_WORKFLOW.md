# RobotFace User Guide Publishing

## Canonical Host

The live RobotFace user guide is hosted by the `phodara/roboface-audio`
GitHub Pages site, not by `roboface-simulator`.

Live URL:

```text
https://phodara.github.io/roboface-audio/docs/user-guide/index.html
```

`roboface-simulator` commits and syncs should continue to go to the
`phodara/roboface-simulator` repository. Publishing the user guide is a
separate deployment step to `phodara/roboface-audio`.

## Index File Map

`index.html` in this repository is the simulator app for
`phodara/roboface-simulator`; it is not the live GitHub Pages file staged on
`roboface-audio`.

The live `roboface-audio` page is `docs/user-guide/index.html` in the
`phodara/roboface-audio` repository.

## Source File

Edit the source guide in the firmware/project workspace:

```text
/Users/paulhodara/Documents/PlatformIO/roboface5/docs/user-guides/print/RobotFace_User_Guide_System_Form_GitHub.html
```

The guide should use relative image paths:

```text
system_form_faces_png/...
```

Do not point guide images at `raw.githubusercontent.com` for the canonical live
page. The HTML and image folders should live together in `roboface-audio`.

## Publish Target

Copy the guide package into the `roboface-audio` checkout as:

```text
docs/user-guide/index.html
docs/user-guide/system_form_faces_png/
```

Then commit and push from the `roboface-audio` repository.

## Manual Publish Steps

```sh
cd /private/tmp/roboface-audio-upload
git pull --ff-only origin main

mkdir -p docs/user-guide
cp /Users/paulhodara/Documents/PlatformIO/roboface5/docs/user-guides/print/RobotFace_User_Guide_System_Form_GitHub.html docs/user-guide/index.html
cp -R /Users/paulhodara/Documents/PlatformIO/roboface5/docs/user-guides/print/system_form_faces_png docs/user-guide/

rg "raw.githubusercontent.com|https://" docs/user-guide/index.html
git status --short
git add docs/user-guide
git commit -m "Update RobotFace user guide"
git push origin main
```

If the `rg` command prints image URLs, fix the HTML before committing.

## Verify

After pushing, wait for GitHub Pages to build, then check:

```sh
curl -I 'https://phodara.github.io/roboface-audio/docs/user-guide/index.html'
```

Expected result: `HTTP/2 200`.
