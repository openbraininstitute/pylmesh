from skbuild import setup

setup(
    packages=["pylmesh"],
    package_dir={"": "python"},
    cmake_install_dir="python/pylmesh",
)
