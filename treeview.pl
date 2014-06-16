#!/usr/bin/perl

use strict;
use warnings;
use JSON qw(decode_json);
binmode(STDOUT, ":utf8");
use utf8;

# String to draw
my $barvert = "\x{2502}";
my $barhori = "\x{2500}";
my $angle   = "\x{2514}";
my $new     = "\x{251c}";
my $first   = "\x{252c}";

# Config
my $spaces  = 1;
my $dup     = !1;
my $stripre = !0;

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
        print "> [$from]\n";
    } else {
        print ">$mail [$from]\n";
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


