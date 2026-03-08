FROM ps2dev/ps2sdk:latest

# Install dependencies needed by the GCC toolchain
RUN apk add --no-cache \
    mpc \
    mpfr4 \
    gmp \
    make

# Create symlinks for library version compatibility if needed
RUN if [ -f /usr/lib/libmpc.so.4 ] && [ ! -f /usr/lib/libmpc.so.3 ]; then \
      ln -s /usr/lib/libmpc.so.4 /usr/lib/libmpc.so.3; \
    fi

WORKDIR /workspace
