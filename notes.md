This directory is for fixing up the GCS build to work with manylinux1

- current blocker is getting `-DGPR_MANYLINUX1=1` compiler def into the GRPC build
- probably the best solution is to decouple the builds and do the deps ourselves
    - at least: `grpc, c-ares, google-cpp-common, protobuf`
      (see google-cloud-cpp `super/CMakeLists.txt`)


How to build manually:
- docker baseline w/ working starting point is in `TileDB-Py-GCS:ihn/dev-gcs-build`

- manual steps from TileDB-Py dockerfile:
    - set the ENV vars w/ compiler/lib defs
    - run perlbrew lines for openssl 
    - run `$CMAKE ...` line


Links
- https://github.com/grpc/grpc/pull/13517