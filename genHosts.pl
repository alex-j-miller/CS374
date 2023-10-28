#!/usr/bin/perl

# Author: Chris Wieringa
# Date: 2019-09-17
#
# Changelog: 
#  2020-08-21 - add goldvm01 - goldvm10 to the list
#  2019-09-17 - rewritten to be 'smarter' when finding hosts; actually test
#   them for rsh login ability before adding them to the list.
#   Additionally checks for valid .rhosts files
#  2019-09-24 - updated home to use ENV{HOME} instead of assuming /home/ + 
#   ENV{HOME}
#  2021-10-13 - updated to put the current hostname at the top of the list
#   if that host is in the list.  Thanks Jack Westel for the idea.
#  2021-10-25 - allow running from cs-ssh to figure out what machines are
#   available for SSH login.  You still won't be able to run MPI from cs-ssh 
#   Thanks Adam DenHaan for the idea.

use Socket;
use Net::Ping;
use List::Util 'shuffle';
use threads;
use strict;

# # # # # # #
# VARIABLES
# # # # # # #
my $maxtimeout = 10;

# # # # # # #
# MAIN Program
# # # # # # #

# check for proper .rhosts file
my $rhostsfile = sprintf("%s/.rhosts",$ENV{HOME});
if(!(-f $rhostsfile)) { die "Your $rhostsfile does not exist!\nPlease setup .rhosts before running genHosts.pl\n"; }
my $mode = (stat($rhostsfile))[2];
my $modeformatted = sprintf("%04o",$mode & 07777);
if($modeformatted != "0600") { die "Your $rhostsfile has incorrect permissions ($modeformatted)!\nPlease change the .rhosts file permissions to 0600.\n"; }

# read in all the hosts in the .rhosts file
my %rhosts;
open(FILE,$rhostsfile);
foreach my $line (<FILE>) {
  chomp($line);  if(!(defined($rhosts{$line}))) { $rhosts{$line} = 0; }
}
close(FILE);

# check all the hosts in the rhosts file; sping off individual threads
foreach my $host (keys %rhosts) {
  threads->create({'context' => 'scalar'},'threadHost',$host); 
}

# collect all the threads, and create the usablehosts array
my @usablehosts;
my $iterations = 0;
while(threads->list() && $iterations < $maxtimeout) {
  $iterations++;
  sleep(1);

  # check all joinable threads
  foreach my $jthread (threads->list(threads::joinable)) {
    my $msg = $jthread->join();
    if(defined($msg) && $msg ne "") { push(@usablehosts,$msg); }
  }
}

# assume that after 10 seconds, all other threads will fail, so...
foreach my $kthread (threads->list()) {
  $kthread->kill('KILL')->detach;
}

# get local hostname
my $localhostname = `hostname -s`;
chomp($localhostname);

# print a warning if running on cs-ssh
if($localhostname eq 'cs-ssh') { 
  print("# gen-hosts.pl run on cs-ssh.\n# RSH login checks to the machines below were not performed.\n# Use with caution.\n")
}

# print localhostname first if it is there
if(grep(/^$localhostname$/,@usablehosts)) {
  printf("%s\n",$localhostname);
}

# randomize the usablehosts and return it
foreach my $host (shuffle(@usablehosts)) {
  chomp($host);
  next if $host eq $localhostname;
  printf("%s\n",$host);
}


# # # # # #
# Subroutines
# # # # # #

# threadHost($hostname) - process to check each host
# Returns: the hostname if available, else nothing
sub threadHost {
  my $host = shift;
  return if $host eq "";

  # thread deal with SIG{KILL}
  local $SIG{KILL} = sub { threads->exit };

  # verify host can be looked up via dns and isn't nonsense
  my $ip; my $a = inet_aton($host);
  if(defined($a)) { $ip = inet_ntoa($a); }
  unless($ip =~ /153\.106/) { return; }

  # ping the host via rsh service
  if(pingHostRsh($host)) {
    # if on cs-ssh, skip the rsh check
    my $running_host = `hostname -s`;
    chomp($running_host);
    if($running_host eq 'cs-ssh') { return $host; }

    # not on cs-ssh, assume lab machine that I should be able to verify rsh
    else {
      if(checkHostRsh($host)) { return $host; }
      else { return; }
    }
  }
  else { return; }
}

# pingHostRsh($hostname) - ping the given host via tcp, checking the rsh port
# Returns: 1 if pingable, 0 otherwise
sub pingHostRsh {
  my $host = shift;
  return 0 if $host eq "";

  # do the ping
  my $p = Net::Ping->new("tcp",3);
  $p->{port_num} = 514;  # rsh
  $p->service_check(1);
  return $p->ping($host);
}

# checkHostRsh($hostname) - check that rsh is responding appropriately
# Returns: 1 if you can log in, 0 otherwise
sub checkHostRsh {
  my $host = shift;
  return 0 if $host eq "";

  # do the check
  my $output = `rsh $host "mount | grep /home | grep katz" 2>&1`;
  if($output =~ /katz/) { return 1; }
  else { return 0; }
}
