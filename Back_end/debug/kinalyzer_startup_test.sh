#!/bin/bash
cd /opt/kinalyzer/
taskset -cp 31,33 /usr/bin/perl debug/kinalyzer_scheduler_test_1.pl 
