# Mail Query

Query the contents of a Maildir for From: addresses. Print these 
addresses in the format expected by mutt as a query_command similar to 
`abook --mutt-query foo`

### Usage:

    # add to muttrc
    set query_command = "mail-query '%s' ~/Mail/GMail/INBOX"

This is my first anything in C, so be nice.
