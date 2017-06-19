# Sensei

Data collection and analysis tool for ClassInsight.

#### [⌨️ Developing](#-developing-1)
#### [🚀 Deploying](#-deploying-1)
#### [🏃 Running](#-running-1)

## ⌨️ Developing

*NOTE:* This project uses Git submodules. Make sure you clone this repository with `--recursive`, like so:

```
git clone --recursive git@github.com:synergylabs/sensei.git
```

And when you're pulling and someone else has added a submodule, do this:

```
git submodule update --recursive --init
```

### Adding an openFrameworks add-on

```
git submodule add https://github.com/user/ofxFooBar 3rdParty/addons/ofxFooBar
```

### Installing dependencies

This project has a lot of dependencies. You can read `CMakeLists.txt` to see a full list.

Fortunately, we have a Dockerfile which contains an automated installation script for Ubuntu 16.04.

This project has been built under macOS before, but I'd strongly recommend developing on Ubuntu as you won't need to worry about dependencies.


### Building, running, debugging

**Build:** `./build.sh`

**Run:** `./run.sh`

**Debug:** `./run.sh --debug`

Simple!

## 🚀 Deploying

- Commit and push your changes to this repo onto github
- Change `SENSEI_VERSION` in `Dockerfile` (currently lives in [aroman/sensei-docker](https://github.com/aroman/sensei-docker)) to the full hash of the commit you want to deploy. e.g. `4a13ec34d7c0ad3853ecf5d530dc8048c6c70ad8`.
- Rebuild the docker image by running `docker build -t sensei .` (assuming you are running from a directory containing the dockerfile)
- Tag the built image: `docker tag sensei aroman/sensei:<TAG>`, where `<TAG>` is a tag name you've created for the build (e.g. date, commit hash)
- Push the image: `docker push aroman/sensei:<TAG>`
- Pull the image on each machine you want to upgrade: `docker pull aroman/sensei:<TAG>`

**Note:** `docker pull` doesn't work like `git pull` — it doesn't always pull down the latest version. You should use explicit tags when pulling and pushing. See [here](https://medium.com/@mccode/the-misunderstood-docker-tag-latest-af3babfd6375) for a more detailed explanation.

## 🏃 Running

- Run:

```
DISPLAY=:0 xhost +local:root

DISPLAY=:0 docker run -it \
  --privileged \
  -e DISPLAY=:0 \
  -e SENSEI_MODE=record \
  -v /tmp/.X11-unix:/tmp/.X11-unix \
  -v /dev/bus/usb \
  -v /dev/dri \
  aroman/sensei:2c9844c991fd4c6f31f379045a0d045bf095f04d
```
### Extract recordings from pre-NAS recordings (deprecated)

```
sudo docker export CONTAINER_NAME > contents.tar
tar -xf contents.tar usr/src/sensei/install/recordings --strip-components=4
```
