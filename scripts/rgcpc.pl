#! /usr/bin/perl
#
#
# COPYRIGHT UNIVERSITY OF MANCHESTER, 2003
#
# Author: Mark Mc Keown
# mark.mckeown@man.ac.uk
#
# LICENCE TERMS
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#  1. Redistributions of source code must retain the above copyright
#     notice, this list of conditions and the following disclaimer.
#  2. Redistributions in binary form must reproduce the above copyright
#     notice, this list of conditions and the following disclaimer in the
#     documentation and/or other materials provided with the distribution.
#
# THIS MATERIAL IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. THE ENTIRE RISK AS TO THE QUALITY AND PERFORMANCE OF THE
# PROGRAM IS WITH YOU.  SHOULD THE PROGRAM PROVE DEFECTIVE, YOU ASSUME THE
# COST OF ALL NECESSARY SERVICING, REPAIR OR CORRECTION.
#
#
# version 0.2
#
#




#use strict;
use XML::DOM;




#
# This HASH maps machine names to their interface and DN
# Handy when a machine has multiple interfaces where one
# is maybe faster 
#
#Entry for bezier.man.ac.uk
my %MachineKeys = ('bezier.man.ac.uk' => 1 );
#MR:EDIT:HERE
$MachineInterFace->{'bezier.man.ac.uk'}{'Interface'} = "bezier.man.ac.uk";
$MachineInterFace->{'bezier.man.ac.uk'}{'DN'} = '/C=UK/O=eScience/OU=Manchester/L=MC/CN=host/bezier.man.ac.uk/Email=mark.mckeown@man.ac.uk';


#
# These hashs splits the machines used for SC2003 into two groups - those
# in the UK and those in the US. The script will use this info to determine
# which machine to copy files from when it has a choice of machines to
# use.
#
my %USMachines = ( 'lemieux.psc.edu' => 1,
                   'tg-login.uc.teragrid.org' => 1,
		   'tg-viz-login.uc.teragrid.org' => 1,
		   'tg-login.caltech.teragrid.org' => 1,
		   'tg-login.sdsc.teragrid.org' => 1,
		   'tg-login.ncsa.teragrid.org' => 1 );
		   
my %UKMachines = ( 'green.cfs.ac.uk' => 1,
                   'bezier.man.ac.uk' => 1,
		   'dirac.chem.ucl.ac.uk' => 1,
		   'login.hpcx.ac.uk' => 1,
		   'vermont.mvc.mcc.ac.uk' => 1,
		   'methuselah.mvc.mcc.ac.uk' => 1 );	   


# make sure I have a valid proxy
chomp( $grid_proxy_info = `which grid-proxy-info 2>/dev/null`);
die "grid-proxy-info not found, stopped" if (!(-x $grid_proxy_info));
$timeleft = &runcmd("grid-proxy-info -timeleft");
die "grid-proxy-info failed, stopped"
    if (!defined($timeleft) || $timeleft eq "");
die "proxy expired, stopped" if ($timeleft < 60);


#pick up globus-url-copy
chomp(my $globus_url_copy = `which globus-url-copy 2>/dev/null`);
if (!(-x $globus_url_copy)) {
    die "globus-url-copy not found, please add to \$PATH";
}

if (!@ARGV) {
 print "rgcp - RealityGrid CheckPoint copier script version 0.1\n\n";
 print "Usage:\n";
 print "    rgcpc -t host -f CheckPointfile\n";
 print "    rgcpc -t host -g GSH\n"; 
 print "    rgcpc -t host -g GSH -o OutPut\n";
 print "    rgcpc -x -t host -g GSH -o OutPut\n";
 print "\n";
 print "Where:\n";
 print "      host is the target eg. gsiftp://hostname/tmp or file:/tmp\n";
 print "      CheckPointfile is the XML file that has the check point information,\n";
 print "      this can be either a local file or a file hosted on a Web Server.\n";
 print "      GSH is the Grid Service Handle of the CheckPoint Node.\n";
 print "      OutPut is a file where the new CheckPoint data is copied to.\n";
 print "      -x causes the script to force the copy, normally if the files\n";
 print "      are already on the Target machine they are not copied over\n\n";
 print "        rgcpc -t gsiftp://hostname/tmp -f /tmp/blah.xml\n";
 print "        rgcpc -f http://hostname/blah.xml -t gsiftp://hostname/tmp\n";
 print "        rgcpc -f http://hostname/blah.xml -t file:/tmp\n";
 print "        rgcpc -g http://hostname/123123 -t gsiftp://hostname/tmp\n";
 print "\n";
 print "rgcp will also take command line arguments for globus-url-copy eg.\n";
 print "        rgcpc -vb -p 6 http://hostname/blah.xml -t gsiftp://hostname/tmp\n";
 print "\n";
 exit;
}

#Target Machine
my $Target = "";
#local file holding the CP data
my $CPfile = "";
#arguments to be passed to globus0url-copy
my $GSIFTPARGS = "";
#flag to indicate if we are accessing the tree directly
my $GSH_FLAG = "FALSE";
#GSH of the tree node holding the CP data
my $GSH = "";
#flag - do we force the copy if files are on local machine
my $FORCE = "FALSE";
#Output file for CP data after we have done copy - could
#be used to update the tree
my $OutPutFile = "";

while (defined($_ = shift(@ARGV))) {
  if(/^-t/) {
    $Target = shift(@ARGV);
  }elsif (/^-f/) {
    $CPfile = shift(@ARGV);
  }elsif (/^-o/) {
     $OutPutFile = shift(@ARGV);
  } elsif (/^-g/) { 
    $GSH = shift(@ARGV);
    $GSH_FLAG = "TRUE";
  }elsif(/^-x/){ 
    $FORCE = "TRUE";
  } else {
    $GSIFTPARGS .= " ".$_;     #arguments for gsiftp
  }
}


#check we have a target
if ( $Target eq "" )
{
  die "Target host not set\n";
}

#check we have a source
if ( $GSH_FLAG eq "FALSE" && $CPfile eq "" )
{
  die "CheckPoint XML file not set\n";
}


# we are going to use a GSH to the tree to get the CP
# data - $ans will hold the result
my $ans;
if ( $GSH_FLAG  eq "TRUE" )
{
  #need SOAP::Lite to do this
  require SOAP::Lite;

  $ans =  SOAP::Lite
           -> uri("RealityGridTree")                #set the namespace
           -> proxy($GSH)                           #location of service
           -> getCheckPointData();                 #function + args to invoke
 
 #handle errors and exit     
 if ( $ans->fault)
 {
  print "ERROR:: ". $ans->faultcode." ". $ans->faultstring."\n";
  exit;
 }
  
}


#
# Now check which is the best locaton to use for the target
#
# get target hostname - could be the local host!!
my $TargetHostName = "";
if ( $Target =~ m/file:/o )
{
 chomp($TargetHostName = `hostname`);
}
else
{
 $TargetHostName = $Target;
 $TargetHostName =~ s/gsiftp:\/\///;
 my @blah = split(/\//,$TargetHostName);
 $TargetHostName = shift( @blah ); 
}
print "Target= ".$TargetHostName."\n";



#check wiether we areusing a local file (including a file on a Web Server)
#or a GSH from the tree to get the CP data - use DOM to parse data
my $doc = "";
my $parser = new XML::DOM::Parser;
if ( $GSH_FLAG  eq "TRUE" ) 
{
 $doc = $parser->parse($ans->result)
}
else
{
 $doc = $parser->parsefile($CPfile);
}

#
# look through all the locations that have a copy of the
# files and store in an array - we associate the place
# in the array with the node number 
#
my $FileNodes = $doc->getElementsByTagName("Files");
my $NoFileNodes = $FileNodes->getLength;
@HostsWithFiles = ();
for ( my $i = 0; $i < $NoFileNodes; $i++ )
{
  my $hostname = $FileNodes->item($i)->getAttributeNode("location")->getValue;

  #check the files are not already on the target machine
  if(  $TargetHostName =~ m/$hostname/  )
  {
    print "WARNING::Chekpoint files Already on Target Machine\n";
    #if the user forces us we copy the files over again - maybe
    #the originals have become corrupt
    if ( $FORCE eq "FALSE" )
    {
      print "Not Copying Files - to force the copy use the \"-s\" option.\n";
      exit 0;
    }
  }
  @HostsWithFiles = (@HostsWithFiles,$hostname);  
}




#
# Check if the Target machine is in the US or the UK or unknown
#
# $NodeToUse identifies the node in the doc to use - it is
# associated with the position of host in @HostsWithFiles -
# we default it to the first node/host/location
my $NodeToUse = 0;
if ( defined($UKMachines{$TargetHostName})  )
{
  #target machine is in the UK - need to find a location in the UK
  my $counter = 0; 
  foreach my $Host (@HostsWithFiles)
  {
    if ( defined($UKMachines{$Host})  )
    {
      $NodeToUse = $counter;     
    } 
   $counter++; 
  }
}
elsif ( defined($USMachines{$TargetHostName})  )
{
 #target machine is in the US - need to find a location in the US
  my $counter = 0; 
  foreach my $Host (@HostsWithFiles)
  {
    if ( defined($USMachines{$Host})  )
    {
      $NodeToUse = $counter;

    } 
   $counter++; 
  } 
}
else  #do not know where Target machine is - use default
{
 $NodeToUse = 0;
}

#We know which Source we are going to use
print "Source= ".$HostsWithFiles[$NodeToUse]."\n";

#move to the correct node in the Document
my $nodes = $FileNodes->item($NodeToUse)->getElementsByTagName("file");
my $n = $nodes->getLength;

#we need the names of the files we are copying over
my ( @files,@targetfilename); 
for ( my $i = 0; $i < $n; $i++ )
{
  my $node = $nodes->item($i);
  my $file = $node->getFirstChild->getNodeValue;
  for ($file)  #get rid of whitespace
  {
   s/^\s+//;
   s/\s+$//;
  }
  @files = (@files, $file);
  my @filenames = split(/\//,$file);  #get the actual file name
  my $filename = @filenames[-1];
  @targetfilename = (@targetfilename,$filename); 
}  

#need a copy of Target
my $SavedTarget = $Target;

#Check the target machine does not have different Interface
# - use the $MachineInterFace HASH
foreach my $key (keys %MachineKeys)
{
  if ( $Target =~ m/$key/g )
  {
    print "Using Alternative Interface for Target - ".$MachineInterFace->{$key}{'Interface'}."\n";
    $Target =~ s/$key/$MachineInterFace->{$key}{Interface}/o;
    $GSIFTPARGS .= " -ds \"".$MachineInterFace->{$key}{'DN'}."\" "; 
  }
}


#
# This is the new node to be added to the checkpoint meta data stuff
# weare not going to use DOM to do this - just simple text manitpulation
my $NewNode = "<Files location=\"$TargetHostName\">\n";

#foreach file copy it to the target location
foreach my $file (@files) {
   my $targetfilename = shift @targetfilename;
   my $AddArgs = "";
   #check we do not have a better interface for the machine
   foreach my $key (keys %MachineKeys)
   {
     if ( $file =~ m/$key/og )
     {
       print "\nUsing Alternative Interface for Source - ".$MachineInterFace->{$key}{'Interface'}."\n";
       $file =~ s/$key/$MachineInterFace->{$key}{Interface}/o;
       $AddArgs = " -ss \"".$MachineInterFace->{$key}{'DN'}."\" ";
     }   
   }
   
   #line to be added to the new CP data 
   $NewNode .="<file type=\"gsiftp-URL\">".$SavedTarget."/".$targetfilename."</file>\n";
      
   print "globus-url-copy $GSIFTPARGS $AddArgs $file ".$Target."/".$targetfilename."\n";
   my $command = "$globus_url_copy $GSIFTPARGS  $AddArgs $file ".$Target."/".$targetfilename;
   
   #check for errors from the globus-url-copy command - if there are any exit
   my $out = runcmd($command);
   if ( ($out =~ m/Process Timed Out/o) || ($out =~ m/rror/o) )
   {
     die "ERROR>>>>>>$out<<<<<<";
   }
}

#Close the node 
$NewNode .= "</Files>\n";

#put the document into a string so we can manipulate it.
my $CPmetadata = $doc->toString;
#strip of the trailing <\/Checkpoint_data>
$CPmetadata =~ s/<\/Checkpoint_data>//o;
#add the new node and the trailing <\/Checkpoint_data>
$CPmetadata .=  $NewNode."</Checkpoint_data>";


#if the CP data was accessed through the GSH and the Tree
#then update the CP data in the tree
if ( ($GSH_FLAG  eq "TRUE") &&  ($FORCE eq "FALSE") )
{
 require SOAP::Lite;
 $ans =  SOAP::Lite
           -> uri("RealityGridTree")                #set the namespace
           -> proxy($GSH)                           #location of service
           -> setCheckPointData($CPmetadata);       #function + args to invoke
 
 if ( $ans->fault)
 {
  print "ERROR:: ". $ans->faultcode." ". $ans->faultstring."\n";
  exit;
 }
  
 print  $ans->result."\n";
}


#if the user requested an output file then print it
if ( $OutPutFile ne "" )
{
  open(OUT, ">$OutPutFile") || die;
  print OUT "$CPmetadata\n";
}


print "Transfer Complete\n";



sub runcmd {
    local ($cmd) = @_;
    alarm 300;
    my $timeout_msg = "Process Timed Out\n";
    my $childkilled = 0;
    my $childpid = open(CMD, "exec $cmd 2>&1 |");
    return "failed to run \"$cmd\"\n" if (!defined($childpid));
    local(@output) = <CMD>;
    local($output) = join("\n", @output);
    alarm 0;
    close(CMD);
#    $output .= $timeout_msg if ($childkilled);
    $output = $timeout_msg if ($childkilled);
    return $output;
}

