#!/usr/bin/perl -w

use strict;
use warnings;


my $file = 'bigfile.txt';
my $server_dir = '../server/';
my $server = './triviald';
my $client = '../client/trivial';
my $port = 4001;

my $error_exist = "TFTP client and/or server have not been found, try running make";

die $error_exist unless (-e "$server_dir/$server" and -e $client);

 print "Running server on port $port...\n";

 system ("cd $server_dir && $server -p $port &") ;

 print "Running client : sending $file to localhost on port $port\n";
 
 if (-e '../server/'.$file){
	 system("killall triviald"); 
	 die "File already exists in server directory.. please remove it"; 
 }

 system ("$client -f $file -w -p $port -H 127.0.0.1");

 if(-e '../server/'.$file){   
	print "The file is now available in the server/ directory\n";
  }

 else {die "An error has occured\n"; }
