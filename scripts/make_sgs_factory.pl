#! /usr/bin/env perl

use LWP::Simple;
#use SOAP::Lite +trace =>  debug => sub {};
use SOAP::Lite;
use XML::Parser;

if( @ARGV != 2  )
{
  print "Usage: make_sgs_factory.pl <Address of container> <GSH of top-level registry>\n";
  exit;
}

$SGS_factory = $ARGV[0] . "SGS/factory";
my $registry_GSH = $ARGV[1];

#----------------------------------------------------------------------
# Get factory registry

# Set-up parser for xml returned from querying registries
my $parser = new XML::Parser(ErrorContext => 2);
$parser->setHandlers(Char => \&char_handler,
		     Start => \&start_handler);

my $factory_registry_GSH = "";

# Query registry for registry of factories...
query_registry($registry_GSH, "ogsi:entry");

if(@{gsh_array} > 0){

    #print "Searching entries from registry for factory registry:\n";
    for($i=0; $i<@{gsh_array}; $i++){
	#print "$i: GSH = $gsh_array[$i], Content = $content_array[$i]\n";

	if($content_array[$i] eq "factoryRegistry"){
	    $factory_registry_GSH = $gsh_array[$i];
	    last;
	}
    }
}

# Check that we found a factory registry
if(length($factory_registry_GSH) == 0 ){
    print "\n";
    exit;
}

#----------------------------------------------------------------------
# Get a SGS factory
my $sgs_factory_GSH = get( "$SGS_factory" );
die "Couldn't get $SGS_factory" unless defined $sgs_factory_GSH;

# Register this factory
my $func = "registerSelf";
my $content = "SGSfactory";

my $ans =  SOAP::Lite
    -> uri("factory") #set the namespace
    -> proxy("$sgs_factory_GSH")  #location of service
    -> $func($factory_registry_GSH, $content)
    -> result;

#print "Locator for ServiceGroupEntry = $ans\n";

print "$sgs_factory_GSH\n";

#-------------------------------------------------------------------------

sub char_handler
{
   my ($p, $data) = @_;

   if($store_gsh_flag == 1){

       $gsh_array[$count] = $data;
       $store_gsh_flag = 0;

   } elsif ($store_content_flag == 1) {
# Assume that entry ALWAYS has an associated content element and that it
# comes AFTER the handle.
       $content_array[$count] = $data;
       $store_content_flag = 0;
       $count++;
   }
}

#--------------------------------------------------

sub start_handler
{
   my ($p, $element) = @_;

   if($element eq "ogsi:handle"){

       $store_gsh_flag = 1;

   } elsif($element eq "ogsi:content") {

       $store_content_flag = 1;
   }
}

#---------------------------------------------------

sub query_registry
{
    my($gsh, $search_string) = @_;

    $func = "findServiceData";
    $arg  = "<ogsi:queryByServiceDataNames names=\"".$search_string."\"/>";
    $list = SOAP::Lite
	-> uri("ServiceGroupRegistration")
	-> proxy("$gsh")
	-> $func("$arg")
	-> result;

    #print "\nGot entries from Registry: >>$list<<\n\n";

    $count = 0;
    @gsh_array = ();
    @content_array = ();
    $store_gsh_flag = 0;
    $store_content_flag = 0;

    $parser->parse($list);
}
