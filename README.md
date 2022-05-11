[[_TOC_]]

# overview

**comma** is a generic library for fixed-width (comma-separated or binary) and structured (e.g. json) data processing primarily on Linux. MacOS is supported, but less tested.

**comma** consists of a collection of command line utilities, as well as underlying C++ and python libraries.

**comma** design considerations:
- high throughput for offline data processing
- working with latency-sensitive realtime data streams
- high modularity and separation of concerns
- ease of rapid prototyping and functionality change
- ease of mixing with other applications

You can limit yourself to using the **comma** command line utilities. Further, **comma** provides libraries for serialization, binary protocol packet layouts, etc.

This documentation will have not more than a brief description of each command line utility. For details, each utility has elaborate help, e.g. run: **csv-eval --help**.

Instead, this documentation is intended to provide more and more step-by-step tutorials and recipes for **comma** command line applications.

The library code is self-documented. You can generate it with **doxigen**. Use **git grep** or explore code of respective applications for usage examples.

# installation

## latest release

```
sudo add-apt-repository --yes ppa:orthographic/ppa
sudo add-apt-repository --yes multiverse
sudo apt-get update
sudo apt-get install comma
pip3 install comma-py==1.0.0
```

## build from source
### building with ansible

```
> sudo apt-get install ansible
> mkdir src
> cd src
> git clone https://gitlab.com/orthographic/comma.git
> ansible-playbook comma/system/ansible/install.yml --ask-become-pass
```

### building it manually
#### install dependencies (ubuntu)
```
sudo apt-get install git build-essential cmake cmake-curses-gui perl python3-dev python3-numpy libboost-all-dev socat libzmq3-dev libgtest-dev libprocps-dev recode expat gawk
```
#### build

If you build from source, you will get latest features.

Albeit more tedious, it is safe since **comma** master branch is thoroughly tested and new features are backward-compatible.

```
> mkdir -p src build/comma
> cd src
> git clone https://gitlab.com/orthographic/comma.git
> cd ../build/comma
> cmake ../../src/comma && make && sudo make install
```

Read install.yml, if interested in tweaking build configuration.

# command line applications

- [**csv**: working with comma-separated and fixed-width binary data](csv/overview)
- [**io**: utilities for input/output manipulations](io/overview)
- [**name_value**: working with name-value pair data formats](name_value/overview)

# bash scripting: utilities and design recipes

**comma** applications help with complex data processing when they are bootstrapped together in Linux pipelines or bash scripts.

**comma** bash utilities make writing bash scripts quicker and cleaner.

[more...](bash/overview)

# comma-test-run: regression test framework

comma-test-run is a command line utility that runs minimalistic language-agnostic regression test suits.

[more...](comma-test-run/comma-test-run:-a-simple-regression-test-framework)

