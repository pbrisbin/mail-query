# Maintainer: Patrick Brisbin <pbrisbin@gmail.com>

pkgname=mail-query
pkgver=0.0.1
pkgrel=1
pkgdesc="Parse a maildir and output addresses for use in mutt queries"
arch=('any')
url="https://github.com/pbrisbin/mail-query"
license=('MIT')
source=(${pkgname}.c Makefile)

build() {
  make
}

package() {
  make PREFIX=/usr DESTDIR="$pkgdir" install
}
md5sums=('8dbf1d2e45438419abe683175b6d8d87'
         '8ede780eddfd8be5e8cadb7403a17699')
