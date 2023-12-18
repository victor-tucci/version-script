# Version-script

### Building & Running procedure:

```bash
git clone https://github.com/victor-tucci/version-script.git
cd version-script
mkdir build
cd build
cmake ..
make -j$(nproc)
./bin/versioncheck
```