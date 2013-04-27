# Mail Query

Query the contents of a Maildir for From: addresses. Print these 
addresses in the format expected by mutt as a query_command similar to 
`abook --mutt-query foo`

## Installation

~~~
$ make
$ sudo make install
~~~

A PKGBUILD is present for Arch users.

## Usage:

Add to your `muttrc`:

~~~
set query_command = "mail-query '%s' ~/Mail/INBOX"
~~~
