### Installation

*NOTE:* This project uses Git submodules. Make sure you clone this repository with `--recursive`, like so:

```
$ git clone --recursive git@github.com:synergylabs/sensei.git
```

#### Dependencies

This project has a lot of dependencies. You can read `CMakeLists.txt` to see a full list.

*TODO*: List them here with a command to install all of them on macOS/Ubuntu

#### Compiling

We use (CMake)[https://en.wikipedia.org/wiki/CMake]. Basically it's a build script generator, which can produces Makefiles as well as Xcode project files, etc. If you've never used it before, the dance goes like this:

```
$ cd sensei
$ mkdir build && cd build
$ cmake .. -D CMAKE_INSTALL_PREFIX=../install
$ make
$ make install
```

If that works successfully, you can run the project with `./run.sh` in the top-level. This is basically just executes `install/sensei` (the actual binary executable) but sets `PYTHONPATH` so that mxnet-mtcnn works correctly.

#### Notes for Cyrus

1. You need to patch mxnet_mtcnn to make it single-threaded. This is trivially easy: edit `3rcParty/mxnet_mtcnn/mtcnn_detector.py` and on line 241 change `self.Pool.map` to `map`.

2. You might need to edit `run.sh` a bit based on where your python packages are installed.
