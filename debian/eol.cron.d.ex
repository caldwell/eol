#
# Regular cron jobs for the eol package.
#
0 4	* * *	root	[ -x /usr/bin/eol_maintenance ] && /usr/bin/eol_maintenance
