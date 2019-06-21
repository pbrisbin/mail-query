# Mail Query

Query the contents of a Maildir, say `~/Mail/INBOX`, for `From:` addresses. Print 
these  addresses in the format expected by mutt as a `query_command`, similar to 
`abook --mutt-query foo`

## Installation

```console
$ make
$ sudo make install
```

A PKGBUILD is present for Arch users.

## Usage

Your Maildir being `~/Mail/INBOX`, add to your `muttrc`:

```muttrc
set query_command = "mail-query '%s' ~/Mail/INBOX"
```

To decode [7-bit ASCII encoded MIME-headers](https://tools.ietf.org/html/rfc2047) (starting, for example, with `=?UTF-8?` or `=?ISO-8859-1?`),
ensure that `perl` is executable and the [Encode::MIME:Header](https://perldoc.perl.org/Encode/MIME/Header.html)
module is installed and replace the above line in your `muttrc` by 
```
set query_command= "mail-query '%s' ~/Mail/INBOX |  perl -CS -MEncode -ne 'print decode(\"MIME-Header\", $_)'"
```

## Extensions

The plug-in [vim-mailquery][] lets you complete e-mail addresses inside
vim via mail-query.

[vim-mailquery]: https://github.com/Konfekt/vim-mailquery
