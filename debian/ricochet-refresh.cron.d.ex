#
# Regular cron jobs for the ricochet-refresh package
#
0 4	* * *	root	[ -x /usr/bin/ricochet-refresh_maintenance ] && /usr/bin/ricochet-refresh_maintenance
