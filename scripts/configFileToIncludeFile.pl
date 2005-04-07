#! /usr/bin/env perl

use strict;

if(@{ARGV} != 1){
    print "Usage: configFileToIncludeFile.pl <config_file>\n";
    exit;
}

my $filename = $ARGV[0];

open(CONFIG_FILE, $filename) || die("can't open config file: $!");
print "mConfigFileContent = QString(";
while(<CONFIG_FILE>){
    chomp;
    my $line = $_;

    $line =~ s/"/\\"/og;
    print "\"".$line."\\n\"\n";
}
close(CONFIG_FILE);
print ");\n";


