### Installation

*NOTE:* This project uses Git submodules. Make sure you clone this repository with `--recursive`, like so:

```
git clone --recursive git@github.com:synergylabs/sensei.git
```

And when you're pulling and someone else has added a submodule, do this:

```
git submodule update --recursive --init
```

#### Dependencies

This project has a lot of dependencies. You can read `CMakeLists.txt` to see a full list.

##### Installing `mxnet`

We need to all be on the same version of mxnet, since the API can break.
0.9.3 is what we've settled on for now. To install that particular version:
```sh
cd ~
git clone --recursive --branch v0.9.3 https://github.com/dmlc/mxnet.git
cd mxnet
sh setup-utils/install-mxnet-osx-python.sh
```

##### Installing libfreenect2

We use only the OpenCL depth processing packet pipeline, so need to make sure libfreenect2
is compiled with support for that. Here are the CMake flags you want:

`cmake .. -DENABLE_CXX11=TRUE -DENABLE_OPENCL=TRUE -DENABLE_OPENGL=FALSE`

*TODO*: List them here with a command to install all of them on macOS/Ubuntu

#### Building, running, debugging

**Build:** `./build.sh`
**Run:** `./run.sh`
**Debug:** `./run.sh --debug`

Simple!
