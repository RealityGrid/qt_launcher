#! /usr/bin/env perl

use XML::DOM;
#use SOAP::Lite +trace => debug => sub {};
use SOAP::Lite;

# Instruct a simulation to take a checkpoint and then stop.
# Argument: GSH of the Steering Grid Service
# Returns:  GSH of the newly-created checkpoint

if( @ARGV != 1 ){

  print "Usage: checkpoint_and_stop.pl <GSH of SGS>\n";
  exit;
} 

my $sgs_GSH = $ARGV[0];
my $timeOut = 30;

#-------- We attach... ---------------

$ans=  SOAP::Lite
       -> uri('SGS')
       -> proxy("$sgs_GSH")
       -> Attach()
       -> result;

# print "Attach returned: \n$ans\n";

if($ans eq "SGS_ERROR"){

    print "Failed to attach.\n";
    exit;
}

my $done = 0;
my $count = 0;

while($done != 1){

    sleep 1;

    $ans=  SOAP::Lite
	-> uri('SGS')
	-> proxy("$sgs_GSH")
	-> GetNotifications()
	-> result;

#    print "Got notifications: \n$ans\n";

    if($ans =~ m/SGS:ChkType_defs/){
	$done = 1;
    }

    $count++;
    if($count > $timeOut){
	last;
    }
}

#------- Get the list of checkpoint types -------------

$ans=  SOAP::Lite
       -> uri('SGS')
       -> proxy("$sgs_GSH")
       -> findServiceData(SOAP::Data->value("<ogsi:queryByServiceDataNames names=\"SGS:ChkType_defs\"/>")->type('string'))
       -> result;

#print "findServiceData returned: $ans\n";

if(length($ans) == 0){
    Detach();
    exit;
}

# Use DOM to parse the XML fragment
my $dom_parser = new XML::DOM::Parser;
my $doc = $dom_parser->parse($ans);
my $nodes = $doc->getElementsByTagName("ChkType");
my $NodeSize = $nodes->getLength;

$done = 0;
for($i=0; $i<$NodeSize; $i++){
    my $node = $nodes->item($i)->getElementsByTagName("Direction");
    my $direction = $node->item(0)->getFirstChild->getNodeValue;

    # Find the first checkpoint type with direction INOUT or OUT
    if($direction eq "INOUT" || $direction eq "OUT"){
	$done = 1;
	last;
    }
}

if($done != 1){
    print "Failed to find ChkType with Direction = INOUT\n";
    Detach();
    exit;
}

$node = $nodes->item($i)->getElementsByTagName("Handle");
my $handle = $node->item(0)->getFirstChild->getNodeValue;

# print "Checkpoint handle = $handle\n";

my $ctrlMessage = "<ReG_steer_message xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns=\"http://www.realitygrid.org/xml/steering\" xsi:SchemaLocation=\"http://www.realitygrid.org/xml/steering /home/zzcguap/projects/reg_steer_lib/xml_schema/reg_steer_comm.xsd\"><Steer_control><Command><Cmd_id>" . $handle . "</Cmd_id><Cmd_param><Value>OUT</Value></Cmd_param><Cmd_param><Value>1</Value></Cmd_param></Command></Steer_control></ReG_steer_message>";

$ans=  SOAP::Lite
    -> uri('SGS')
    -> proxy("$sgs_GSH")
    -> PutControl(SOAP::Data->value("$ctrlMessage")->type('string'))
    -> result;

#    print "Control set: \n$ans\n";

#-------- Get the GSH of the latest checkpoint --------

# We have to wait for the app. to tell us it took a checkpoint
# because that's when a node is added to the tree
$done = 0;
$count = 0;

while($done != 1){

    sleep 1;

    $ans=  SOAP::Lite
	-> uri('SGS')
	-> proxy("$sgs_GSH")
	-> GetNotifications()
	-> result;

#    print "Got notifications: \n$ans\n";

# Assume that a change in Application status means it's stopping
# - we could check but that's going to get complicated
    if($ans =~ m/SGS:Chkpoint_GSH/){
	$done = 1;
	last;
    }

    $count++;
    if($count > $timeOut){
	last;
    }
}

if($done != 1){

    print "Failed to get notification of new checkpoint GSH in $timeOut seconds\n";
    Detach();
    exit;
}

$ans=  SOAP::Lite
       -> uri('SGS')
       -> proxy("$sgs_GSH")
       -> findServiceData(SOAP::Data->value("<ogsi:queryByServiceDataNames names=\"SGS:Chkpoint_GSH\"/>")->type('string'))
       -> result;

# print "findServiceData returned: $ans\n";

# Strip locator and handle tags from response to leave GSH
$ans =~ s/<\/?sd:serviceDataValues>//og;
$ans =~ s/<\/?SGS:Chkpoint_GSH>//og;
my $chkGSH = $ans;

#-------- Stop application ----------------------------

$ans=  SOAP::Lite
       -> uri('SGS')
       -> proxy("$sgs_GSH")
       -> Stop()
       -> result;

# Wait for notification that app. has stopped
$done = 0;
$count = 0;

while($done != 1){

    sleep 1;

    $ans=  SOAP::Lite
	-> uri('SGS')
	-> proxy("$sgs_GSH")
	-> GetNotifications()
	-> result;

#    print "Got notifications: \n$ans\n";

# Assume that a change in Application status means it's stopping
# - we could check but that's going to get complicated
    if($ans =~ m/SGS:Application_status/){
	$done = 1;
	last;
    }

    $count++;
    if($count > $timeOut){
	last;
    }
}

if($done != 1){

    print "Application failed to stop in $timeOut seconds!\n";
    print "Attempting detach anyway...\n";
}

#------------- Steerer requests detach... ----------------

Detach();

# If all goes well, our only output is the GSH of the 
# checkpoint just created.
print "$chkGSH\n";

#---------------------------------------------------

sub Detach
{
    my $ans=  SOAP::Lite
	-> uri('SGS')
	-> proxy("$sgs_GSH")
	-> Detach()
	-> result;

#    print "Detach returned: $ans\n";
}
