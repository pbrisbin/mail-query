# Mail Query

Query the contents of a Maildir (e.g. `~/Mail/INBOX`), for `From:` addresses.
Print these  addresses in the format expected by mutt as a `query_command`,
similar to `abook --mutt-query foo`.

## Installation

```console
make
sudo make install
```

A `PKGBUILD` is present for Arch users.

## Usage

Assuming your Maildir is at `~/Mail/INBOX`, add to your `muttrc`:

```muttrc
set query_command = "mail-query '%s' ~/Mail/INBOX"
```

To decode [7-bit ASCII encoded MIME-headers][rfc2047] (starting, for example,
with `=?UTF-8?` or `=?ISO-8859-1?`), ensure that `perl` is executable and the
[`Encode::MIME:Header`][perl-mime-header] module is installed, then replace the
above line in your `muttrc` by:

[rfc2047]: https://tools.ietf.org/html/rfc2047
[perl-mime-header]: https://perldoc.perl.org/Encode/MIME/Header.html

```muttrc
set query_command= "mail-query '%s' ~/Mail/INBOX | perl -CS -MEncode -ne 'print decode(\"MIME-Header\", $_)'"
```

## Extensions

The plug-in [vim-mailquery][] lets you complete e-mail addresses inside vim via
mail-query.

[vim-mailquery]: https://github.com/Konfekt/vim-mailquery
