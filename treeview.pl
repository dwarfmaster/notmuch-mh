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

my $json;
$json  = get_input();
parse_level($barvert, 0, $new, $json);

sub get_input {
    my $text;
    while(<STDIN>) {
        $text = "$text$_";
    }
    if($text =~ m/\[\[(.*)\]\]/) {
        $text = $1;
    }
    return decode_json($text);
}

sub parse_level {
    my ($symb,$dec,$lnew,$json) = @_;
    my ($mail,$from,$to,@subs) = parse_headers($json);

    my $i;
    for($i = 0; $i < $dec - 1; ++$i) {
        print "$symb";
    }
    print "$lnew" if $dec >= 1;
    print "$from -> $to { $mail }\n";

    for($i = 0; $i < scalar(@subs) - 1; ++$i) {
        parse_level($symb,$dec+1,$new,$subs[$i]);
    }
    parse_level(" ",$dec+1,$angle,$subs[-1]) if scalar(@subs) > 0;
}

sub parse_headers {
    my ($json) = @_;
    my $item = @$json[0];

    my $mail = $item->{'headers'}{'Subject'};
    my $from = $item->{'headers'}{'From'};
    my $to   = $item->{'headers'}{'To'};
    my $subs = @$json[1];

    return ($mail, $from, $to, @$subs);
}


