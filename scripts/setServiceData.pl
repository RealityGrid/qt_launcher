#! /usr/bin/env perl

use LWP::Simple;
use SOAP::Lite +trace =>  debug => sub {};
#use SOAP::Lite;
use strict;

if( @ARGV != 3  )
{
  print "Usage: setServiceData.pl <namespace> <GSH> <text>\n";
  exit;
}

my $namespace = $ARGV[0];
my $gsh = $ARGV[1];
my $arg = $ARGV[2];

print "namespace = $namespace\n";
print "gsh       = $gsh\n";
print "arg       >>$arg<<\n";

my $ans = SOAP::Lite
          -> uri("$namespace")
          -> proxy("$gsh")
          -> setServiceData("$arg")
          -> result;

print "setServiceData returned: >>$ans<<\n";

