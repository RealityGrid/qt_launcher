#! /usr/bin/env perl

use LWP::Simple;
#use SOAP::Lite +trace =>  debug => sub {};
use SOAP::Lite;
use XML::DOM;

if( @ARGV < 7  )
{
  print "Usage: make_msgs.pl <GSH of factory> <tag for SGS> <GSH of registry> <GSH of checkpoint> <input file> <run time (min)> <GSH of parent> [<Tag for checkpoint tree> <GSH of checkpoint tree factory>]\n";
  exit;
}

my $sgs_factory_GSH = $ARGV[0];
my $content = $ARGV[1];
my $registry_GSH = $ARGV[2];
my $chkGSH = $ARGV[3];
my $input_file = $ARGV[4];
my $run_time = $ARGV[5];
my $parentGSH = $ARGV[6];

#----------------------------------------------------------------------
# Check that we have a valid GSH for the checkpoint - if not make a new
# tree provided that a tag has been supplied for it.

# A GSH must have at least 'http' in it
if(length($chkGSH) < 6){

    if( @ARGV == 9  ){
	my $tree_meta_data = $ARGV[7];
	my $target = $ARGV[8];
  
	$uri = "factory";
	$timeToLive = "<ogsi:terminationTime />";

	$result = SOAP::Lite
	    -> uri($uri)              #set the namespace
	    -> proxy("$target")       #location of service
	    -> createNewTree($timeToLive, "", "", $tree_meta_data)
	    -> result;

	# Strip locator and handle tags from response to leave GSH
	$result =~ s/<\/?ogsi:locator>//og;
	$result =~ s/<\/?ogsi:handle>//og;
	$chkGSH = $result;
    }
}

#----------------------------------------------------------------------
# Create MetaSGS

$target = $sgs_factory_GSH;
$uri = "factory";

# Get the time and date
my $time_now = time;

# Give the GS an initial lifetime of 24 hours
($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst) = 
                                         gmtime($time_now + 24*60*60);
$timeStr = sprintf "%4d-%02d-%02dT%02d:%02d:%02dZ",
                     $year+1900,$mon+1,$mday,$hour,$min,$sec;
$timeToLive = "<ogsi:terminationTime after=\"".$timeStr."\"/>";

$result =  SOAP::Lite
                 -> uri($uri)              #set the namespace
                 -> proxy("$target")       #location of service
                 -> createService($timeToLive, $registry_GSH, $content, $chkGSH)
                 -> result;

# Use DOM to parse the XML fragment 
my $parser = new XML::DOM::Parser;
my $doc = $parser->parse($result);
my $node = $doc->getElementsByTagName("ogsi:handle");
my $sgs_GSH = $node->item(0)->getFirstChild->getNodeValue;

#---------------------------------------------------------------------
# Supply input file & max. run time

$content = "";
# Allow 5 more minutes than specified, just to be on the safe side.
$run_time += 5;

if( open(GSH_FILE, $input_file) ){

  while ($line_text = <GSH_FILE>) {
      $content = $content . $line_text;
  }
  close(GSH_FILE);

  # Protect input-file content by putting it in a CDATA section - we 
  # don't want parser to attempt to parse it 
  # Configure the SGS with the max. run-time of the job (is used to 
  # control life-time of the service).
  $content = "<ogsi:setByServiceDataNames><SGS:Input_file><![CDATA[" . $content .
             "]]></SGS:Input_file><SGS:Max_run_time>" . $run_time . 
	     "</SGS:Max_run_time><MSGS:Num_Children>0".
             "</MSGS:Num_Children><MSGS:Parent>".$parentGSH.
             "</MSGS:Parent></ogsi:setByServiceDataNames>";
}
else{
  $content = "<ogsi:setByServiceDataNames><SGS:Input_file></SGS:Input_file>".
             "<SGS:Max_run_time>" . $run_time . "</SGS:Max_run_time>".
	     "<MSGS:Num_Children>0</MSGS:Num_Children><MSGS:Parent>".$parentGSH.
             "</MSGS:Parent></ogsi:setByServiceDataNames>";
}

$target = $sgs_GSH;
$uri = "MetaSGS";

$result =  SOAP::Lite
    -> uri($uri)              #set the namespace
    -> proxy("$target")       #location of service
    -> setServiceData($content)
    -> result;

#----------------------------
# Add child to parent

if( $parentGSH =~ m/http/ ){
    
    $result =  SOAP::Lite
	-> uri("MetaSGS")
	-> proxy("$parentGSH")
	-> addChild($target)
	-> result;
    
#    print "addChild returned: $result\n";
}

#---------------------------------------------------------------------

print "$sgs_GSH\n";
