* Testing *

Testin' is developin'!

To run the tests in this directory, use this setup procedure:

1. Check out a copy of the code from github
1. Ensure that you have PHP installed locally.
1. Ensure that you have access to the `xmllint` program.
1. Serve the PHP files locally. If you're using a recent PHP, this is just
`php -s localhost:8080`
otherwise you may have to fiddle with apache.
1. In another terminal, `cd testing`
1. `./test-ab-initios-setup.sh http://localhost:8080`

This will run tests which assume a brand new setup.
