FROM quay.io/pypa/manylinux2014_x86_64:2023-04-02

# Build with:
# > docker build -f scripts/ci/manylinux/Dockerfile .
# or using buildx:
# > docker buildx build --load --platform linux/amd64 -t tiledb-manylinux2014_x86_64 -f scripts/ci/manylinux/Dockerfile --builder=container .

RUN yum update && \
  yum install -y \
  sudo \
  zip \
  glibc-devel \
  perl-IPC-Cmd
