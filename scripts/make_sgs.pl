#! /usr/bin/env perl

use LWP::Simple;
#use SOAP::Lite +trace =>  debug => sub {};
use SOAP::Lite;
use XML::DOM;

if( @ARGV < 5  )
{
  print "Usage: make_sgs.pl <GSH of factory> <tag for SGS> <GSH of registry> <GSH of checkpoint> <input file> [<Tag for checkpoint tree>]\n";
  exit;
}

my $sgs_factory_GSH = $ARGV[0];
my $content = $ARGV[1];
my $registry_GSH = $ARGV[2];
my $chkGSH = $ARGV[3];
my $input_file = $ARGV[4];

#----------------------------------------------------------------------
# Check that we have a valid GSH for the checkpoint - if not make a new
# tree provided that a tag has been supplied for it.

# A GSH must have at least 'http' in it
if(length($chkGSH) < 5){

    if( @ARGV == 6  ){
	my $tree_meta_data = $ARGV[5];

	$target = "http://vermont.mvc.mcc.ac.uk:50000/Session/RealityGridTree/factory";
	$uri = "factory";
	$func = "createNewTree";
	$timeToLive = "<ogsi:terminationTime />";

	$result = SOAP::Lite
	    -> uri($uri)              #set the namespace
	    -> proxy("$target")       #location of service
	    -> $func($timeToLive, "", "", $tree_meta_data)
	    -> result;

	# Strip locator and handle tags from response to leave GSH
	$result =~ s/<\/?ogsi:locator>//og;
	$result =~ s/<\/?ogsi:handle>//og;
	$chkGSH = $result;
#<ogsi:locator><ogsi:handle>http://vermont.mvc.mcc.ac.uk:50000/Session/RealityGridTree/service?/45825</ogsi:handle></ogsi:locator>
    }
}

#----------------------------------------------------------------------
# Create SGS

$target = $sgs_factory_GSH;
$uri = "factory";
$func = "createService";
$timeToLive = "<ogsi:terminationTime after=\"infinity\"/>";

$result =  SOAP::Lite
                 -> uri($uri)              #set the namespace
                 -> proxy("$target")       #location of service
                 -> $func($timeToLive, $registry_GSH, $content, $chkGSH)
                 -> result;

# Use DOM to parse the XML fragment 
my $parser = new XML::DOM::Parser;
my $doc = $parser->parse($result);
my $node = $doc->getElementsByTagName("ogsi:handle");
my $sgs_GSH = $node->item(0)->getFirstChild->getNodeValue;

#---------------------------------------------------------------------
# Supply input file

open(GSH_FILE, $input_file) || die("can't open input file: $input_file");
$content = "";
while ($line_text = <GSH_FILE>) {
    $content = $content . $line_text;
}
close(GSH_FILE);

$target = $sgs_GSH;
$uri = "SGS";
$func = "setServiceData";
# Protect input-file content by putting it in a CDATA section - we 
# don't want parser to attempt to parse it 
$content = "<ogsi:setByServiceDataNames><SGS:Input_file><![CDATA[" . $content .
    "]]></SGS:Input_file></ogsi:setByServiceDataNames>";

$result =  SOAP::Lite
                 -> uri($uri)              #set the namespace
                 -> proxy("$target")       #location of service
                 -> $func($content)
                 -> result;

#print "setServiceData returned: $result\n";

#---------------------------------------------------------------------

print "$sgs_GSH\n";
