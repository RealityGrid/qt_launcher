#! /usr/bin/env perl

use LWP::Simple;
#use SOAP::Lite +trace =>  debug => sub {};
use SOAP::Lite;
use XML::Parser;
use XML::DOM;


if( @ARGV != 4  )
{
  print "Usage: make_vis_sgs.pl <GSH of factory> <tag for SGS> <GSH of registry> <GSH of data source>\n";
  exit;
}

my $sgs_factory_GSH = $ARGV[0];
my $content = $ARGV[1];
my $registry_GSH = $ARGV[2];
my $source_GSH = $ARGV[3];

#----------------------------------------------------------------------
# Create SGS

$target = $sgs_factory_GSH;
$uri = "factory";
$func = "createService";
$timeToLive = "<ogsi:terminationTime after=\"infinity\"/>";

$result =  SOAP::Lite
                 -> uri($uri)              #set the namespace
                 -> proxy("$target")       #location of service
                 -> $func($timeToLive, $registry_GSH, $content)
                 -> result;

# Use DOM to parse the XML fragment 
my $parser = new XML::DOM::Parser;
my $doc = $parser->parse($result);
my $node = $doc->getElementsByTagName("ogsi:handle");
my $sgs_GSH = $node->item(0)->getFirstChild->getNodeValue;

#-------------------------------------------------------------------------
# Query App. SGS for IOType definitions

$func = "findServiceData";
my $arg = "<ogsi:queryByServiceDataNames names=\"SGS:IOType_defs\"/>";

my $iotypes = SOAP::Lite
              -> uri("SGS")
              -> proxy("$source_GSH")
              -> $func("$arg")
              -> result;

#print "findServiceData returned: >>$iotypes<<\n";

#-------------------------------------------------------------------------
# Parse the IOtypes

my $parser = new XML::Parser(ErrorContext => 2);
$parser->setHandlers(Char => \&iotype_char_handler,
		     Start => \&iotype_start_handler,
		     End => \&iotype_end_handler);

$count = 0;
my @iotype_label_array = ();
my @iotype_dirn_array = ();
my @iotype_address_array = ();
my $is_label = 0;
my $is_direction = 0;
my $is_address = 0;

$parser->parse($iotypes);

#for($i=0; $i<$count; $i++){
#    print "$i: label = $iotype_label_array[$i]\n" .
#          "     Dir = $iotype_dirn_array[$i]\n" .
#          " address = $iotype_address_array[$i]\n";
#}

# Choose a data source automatically for now - look for
# an IOType with direction OUT and a valid address.
my $source_label = "";
for($i=0; $i<$count; $i++){

    if($iotype_dirn_array[$i] eq "OUT"){

	if(index($iotype_address_array[$i], ":") >= 0){

	    $source_label = $iotype_label_array[$i];
	    last;
	}
    }
}

die "Failed to find a valid data source\n" unless (length($source_label) > 0);

#-------------------------------------------------------------------------
# Set-up Vis. SGS with data sources

my $dataSources = "<SGS:Data_source_list>
<SGS:Data_source>
<SGS:Source_GSH>" . $source_GSH . "</SGS:Source_GSH>
<SGS:Source_label>" . $source_label . "</SGS:Source_label>
</SGS:Data_source>
</SGS:Data_source_list>";

$func = "setServiceData";
$arg = "<ogsi:setByServiceDataNames>".$dataSources.
    "</ogsi:setByServiceDataNames>";
my $ans = SOAP::Lite
          -> uri("SGS")
          -> proxy("$sgs_GSH")
          -> $func("$arg")
          -> result;

#print "setServiceData for Data_source_list returned: >>$ans<<\n";

print "$sgs_GSH\n";

#--------------------------------------------------

sub iotype_char_handler
{
   my ($p, $data) = @_;

   if($is_label == 1) {

       $iotype_label_array[$count] = $data;
       $is_label = 0;

   }elsif($is_direction == 1){

       $iotype_dirn_array[$count] = $data;
       $is_direction = 0;

   }elsif($is_address == 1){

       $iotype_address_array[$count] = $data;
       $is_address = 0;

   }
}

#--------------------------------------------------

sub iotype_start_handler
{
   my ($p, $element) = @_;

   if($element eq "Label"){

       $is_label = 1;

   } elsif($element eq "Direction") {

       $is_direction = 1;

   } elsif($element eq "Address"){

       $is_address = 1;
   }
}

#--------------------------------------------------

sub iotype_end_handler
{
   my ($p, $element) = @_;

   if($element eq "IOType"){
       $count++;
   }
}
