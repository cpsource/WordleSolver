#!/bin/bash
sqlite3 ~/snap/firefox/common/.mozilla/firefox/7fyzsdsd.default-release/webappsstore.sqlite <<EOF
select * from webappsstore2 where key = "nyt-wordle-state";
EOF



