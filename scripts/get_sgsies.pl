#! /usr/bin/env perl

use LWP::Simple;
#use SOAP::Lite +trace =>  debug => sub {};
use SOAP::Lite;
use XML::Parser;

if( @ARGV != 1  )
{
  print "Usage: get_sgsies.pl <GSH of top-level registry>\n";
  exit;
}

my $registry_GSH = $ARGV[0];
open(SGS_FILE, "> sgs_list.txt") || die("can't open datafile: $!");

#----------------------------------------------------------------------
# Get list of SGS factories

# Set-up parser for xml returned from querying registries
my $parser = new XML::Parser(ErrorContext => 2);
$parser->setHandlers(Char => \&char_handler,
		     Start => \&start_handler,
                     End => \&end_handler);

# Query registry 
query_registry($registry_GSH, "ogsi:entry");

if(@{gsh_array} > 0){

    #print "Searching entries from registry for factory registry:\n";
    for($i=0; $i<@{gsh_array}; $i++){
	#print "$i: GSH = $gsh_array[$i], Content = $content_array[$i]\n";

	if($content_array[$i] ne "factoryRegistry"){

            # Remove new-line characters that may or may not be present 
            # in the job meta-data
	    $content_array[$i] =~ s/\n/ /og;
	    print SGS_FILE "$gsh_array[$i] $content_array[$i]\n";
	    print "$gsh_array[$i] $content_array[$i]\n";
	}
    }
} 

close(SGS_FILE);

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
# Content now has children so concatenate their contents
       $content_array[$count] = $content_array[$count].$data;
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

#--------------------------------------------------

sub end_handler
{
   my ($p, $element) = @_;

   # Catch end of Content element 
   if($element eq "ogsi:content"){

       $store_content_flag = 0;
       $count++;
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
