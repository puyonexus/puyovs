pkgname=puyovs
pkgver=32
pkgrel=1
pkgdesc='An online puzzle game.'
arch=('x86_64')
url='https://puyovs.com/'
license=('MIT')
makedepends=('cmake' 'gcc' 'make')
depends=('libx11' 'alsa-lib' 'libpulse' 'qt5-base')
source=("${pkgname}::git+https://github.com/puyonexus/puyovs.git")
md5sums=('SKIP')

build() {
    cmake -B build -S "${pkgname}" \
        -DCMAKE_BUILD_TYPE='None' \
        -DCMAKE_INSTALL_PREFIX='/usr' \
        -Wno-dev
    cmake --build build
}

package() {
    DESTDIR="$pkgdir" cmake --install build
}
