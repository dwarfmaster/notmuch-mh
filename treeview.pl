#!/usr/bin/perl

use strict;
use warnings;
use JSON qw(decode_json);
use Getopt::Std;
use Term::ANSIColor;
use utf8;

$Term::ANSIColor::AUTORESET=1;
binmode(STDOUT, ":utf8");

# Config
my %opts;
getopt("Ds", \%opts);
my $spaces  = $opts{"s"};
$spaces = 1 if not $spaces;
my $dup     = $opts{"d"};
my $stripre = $opts{"r"};
my $mdepth  = $opts{'D'};
my $ascii   = $opts{'A'};

# String to draw
my $barvert = "|";
my $barhori = "-";
my $angle   = "\\";
my $new     = "|";
my $first   = "+";
if(not $ascii) {
    $barvert = "\x{2502}";
    $barhori = "\x{2500}";
    $angle   = "\x{2514}";
    $new     = "\x{251c}";
    $first   = "\x{252c}";
}

my $json;
$json  = get_input();
foreach my $thread (@$json) {
    parse_level(0, $new, "", @$thread[0]);
}

sub get_input {
    my $text;
    while(<STDIN>) {
        $text = "$text$_";
    }
    return decode_json($text);
}

sub parse_level {
    my ($dec,$lnew,$prev,$json,@symbs) = @_;
    return if $mdepth and $dec >= $mdepth;
    my ($mail,$from,$to,@subs) = parse_headers($json);

    my $i;
    for($i = 0; $i < $dec - 1; ++$i) {
        print "$symbs[$i]";
        for(my $j = 0; $j < $spaces; ++$j) {
            print " ";
        }
    }

    print "$lnew" if $dec >= 1;
    if(scalar(@symbs) > 0) {
        for(my $j = 0; $j < $spaces; ++$j) {
            print "$barhori";
        }
    }
    if(scalar(@subs) > 0) {
        print "$first";
    } else {
        print "$barhori";
    }
    if(not $dup and $mail eq $prev) {
        print "> ";
        print colored ['green'], "[$from]\n";
    } else {
        print ">";
        print colored ['bold cyan'], "$mail";
        print colored ['green'], " [$from]\n";
    }

    push @symbs, $barvert;
    for($i = 0; $i < scalar(@subs) - 1; ++$i) {
        parse_level($dec+1,$new,$mail,$subs[$i],@symbs);
    }
    $symbs[-1] = " ";
    parse_level($dec+1,$angle,$mail,$subs[-1],@symbs) if scalar(@subs) > 0;
}

sub parse_headers {
    my ($json) = @_;
    my $item = @$json[0];

    my $mail = $item->{'headers'}{'Subject'};
    $mail = strip_re($mail) if $stripre;
    my $from = $item->{'headers'}{'From'};
    my $to   = $item->{'headers'}{'To'};
    my $subs = @$json[1];

    return ($mail, $from, $to, @$subs);
}

sub strip_re {
    my ($mail) = @_;
    while($mail =~ m/^Re: (.*)$/ or $mail =~ m/^Re:(.*)$/) {
        $mail = $1;
    }
    return $mail;
}


