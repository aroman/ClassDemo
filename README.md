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
- Tag the built image: `docker tag sensei aroman/sensei:HASH`, where `HASH` is the commit-hash you are deploying
- Push the image: `docker push aroman/sensei:HASH`
- Pull the image on each machine you want to upgrade: `docker pull aroman/sensei:latest`

## 🏃 Running

- Pull the latest image: `docker pull aroman/sensei:latest`
- Allow X11 connections from docker: `xhost +local:root`
- Run:

```
docker run -it \
  --privileged \
  -e DISPLAY \
  -v /tmp/.X11-unix:/tmp/.X11-unix \
  -v /dev/bus/usb \
  -v /dev/dri \
  aroman/sensei
```
