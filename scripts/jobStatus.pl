#! /usr/bin/env perl

use XML::Parser;
use LWP::Simple;
#use SOAP::Lite +trace =>  debug => sub {};
use SOAP::Lite;

if( @ARGV != 2  )
{
  print "Usage: jobStatus.pl <namespace of service> <GSH of job>\n";
  exit;
}

my $namespace = $ARGV[0];
my $app_SGS_GSH = $ARGV[1];

#------------------------------------------------------------------------
# Query Registry for SGSs

my $arg  = "<ogsi:queryByServiceDataNames names=\"SGS:Application_status\"/>";
my $job_status = SOAP::Lite
               -> uri("$namespace")
               -> proxy("$app_SGS_GSH")
               -> findServiceData("$arg")
               -> result;

print "findServiceData returned: >>$job_status<<\n";

