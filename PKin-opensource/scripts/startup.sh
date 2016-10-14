echo starting short pkin short scheduler
at now -f pkSchedulerShort.sh

echo starting medium pkin scheduler
at now -f pkSchedulerMedium.sh

echo starting long pkin scheduler
at now -f pkSchedulerLong.sh

echo starting extralong pkin scheduler
at now -f pkSchedulerExtraLong.sh

echo starting heartbeat
at now -f heartbeat.sh

echo finished.

