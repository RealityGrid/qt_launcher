#! /usr/bin/env perl

use XML::Parser;
use LWP::Simple;
#use SOAP::Lite +trace =>  debug => sub {};
use SOAP::Lite;

if( @ARGV != 1  )
{
  print "Usage: jobStatus.pl <GSH of job>\n";
  exit;
}

#my $SGS_factory = $ARGV[0] . "SGS/factory";
my $app_SGS_GSH = $ARGV[0];

#------------------------------------------------------------------------
# Query Registry for SGSs

my $func = "findServiceData";
my $arg  = "<ogsi:queryByServiceDataNames names=\"SGS:Application_status\"/>";
my $job_status = SOAP::Lite
               -> uri("SGS")
               -> proxy("$app_SGS_GSH")
               -> $func("$arg")
               -> result;

print "findServiceData returned: >>$job_status<<\n";

