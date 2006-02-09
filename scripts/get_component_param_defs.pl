#! /usr/bin/env perl

use LWP::Simple;
#use SOAP::Lite +trace =>  debug => sub {};
use SOAP::Lite;
use XML::DOM;
use strict;

if( @ARGV != 1 ){
  print "Usage: get_component_param_defs.pl <GSH of parent>\n";
  exit;
}

my $parentGSH = $ARGV[0];
my $content = "<ogsi:queryByServiceDataNames names=\"SGS:Param_defs\"/>";
my %child_params = ();
my @childGSHList = ();
my @file_names = ();
my $parser = new XML::DOM::Parser;

# Get parameter definitions from parent and parse
my $param_xml = SOAP::Lite
                       -> uri("MetaSGS")       #set the namespace
                       -> proxy("$parentGSH")  #location of service
                       -> findServiceData(SOAP::Data->value("$content")->type('string'))
                       -> result;
if (!$param_xml){
    print "ERROR: Got no parameter definitions from parent\n";
    exit;
}
my $param_doc = $parser->parse($param_xml);

# Now get details of parent's children
$content = "<ogsi:queryByServiceDataNames names=\"MSGS:Children\"/>";

my $ans =  SOAP::Lite
    -> uri("MetaSGS")       #set the namespace
    -> proxy("$parentGSH")  #location of service
    -> findServiceData(SOAP::Data->value("$content")->type('string'))
    -> result;

my $doc = $parser->parse($ans);
if(!defined($doc)){
    print "ERROR: Failed to parse children SDE: $ans\n";
    exit;
}

my @nodes = $doc->getElementsByTagName("Child");
if(!@{nodes}){
    print "ERROR: We don't seem to have any children :-(\n";
    exit;
}

# Loop over each child
foreach my $node (@nodes){

    my $childGSH = $node->getFirstChild->getData;
    #print "GSH: $childGSH\n";
    push @childGSHList, $childGSH;

    my @bits = split("/", $childGSH);
    my $childID = pop @bits;

    # Get the name of the application making up this component
    my $name =  SOAP::Lite
	-> uri("MetaSGS")      #set the namespace
	-> proxy("$childGSH")  #location of service
	-> findServiceData(SOAP::Data->value("<ogsi:queryByServiceDataNames ".
			   "names=\"SGS:Application_name\"/>")->type('string'))
	-> result;

    if(index($name,"<sd:serviceDataValues></sd:serviceDataValues>") > -1){
	print "ERROR: Got no application name from child $childGSH\n";
	exit;
    }

    $name =~ s/.*<SGS:Application_name>//og;
    $name =~ s/<\/SGS:Application_name>.*//og;

    # Create a file with name constructed from application name
    my $file_name = $name;
    $file_name =~ s/ /_/og;
    $file_name .= "_metadata_".$childID.".xml";
    push @file_names, $file_name;
    open(PARAM_FILE, "> $file_name") || die("can't open datafile: $!");
    print PARAM_FILE "<Param_defs GSH=\"".$childGSH."\">\n";

    my @params = $param_doc->getElementsByTagName("Param");

    foreach my $param_node (@params){

	# We don't want 'internal' parameters
	my $parent = $param_node->getParentNode();
	my $internal = $param_node->getElementsByTagName("Is_internal");
	my $val = $internal->item(0)->getFirstChild->getData;
	if ("$val" eq "TRUE"){
	    $parent->removeChild($param_node);
	    next;
	}

	# We don't want 'monitored' parameters
        $internal = $param_node->getElementsByTagName("Steerable");
	$val = $internal->item(0)->getFirstChild->getData;
	if ("$val" eq "0"){
	    $parent->removeChild($param_node);
	    next;
	}

        # Check that this parameter belongs to the current child
        # - uses the fact that GSH ID is prepended to label
	$internal = $param_node->getElementsByTagName("Label");
	$val = $internal->item(0)->getFirstChild->getData;
	if(index($val, $childID) <= -1){
	    next;
	}

	my $param = $param_node->toString;
	print PARAM_FILE "$param\n";
    }

#    my @labels = $param_doc->getElementsByTagName("Label");
#
#    my @list = ();
#    foreach my $label_node (@labels){
#
#	my $label = $label_node->getFirstChild->getData;
#	if(index($label, $childID) > -1){
#	    # This parameter belongs to this child - we use fact
#	    # that encoded labels contain the UID from the end
#	    # of a child's GSH.
#	    push @list, $label;
#	}
#    }
#    $child_params{$childGSH} = \@list;

    print PARAM_FILE "</Param_defs>\n";
    close(PARAM_FILE);
}

foreach my $name (@file_names){
    print "$name ";
}
print "\n";


