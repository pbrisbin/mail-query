# Mail Query

Query the contents of a Maildir for From: addresses. Print these 
addresses in the format expected by mutt as a query_command similar to 
`abook --mutt-query foo`

*Rewritten by [falconindy](https://github.com/falconindy/mail-query) on 
[12/22/2010](https://github.com/falconindy/mail-query)*

### Usage:

    # add to muttrc
    set query_command = "mail-query '%s' ~/Mail/GMail/INBOX"
