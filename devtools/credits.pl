#!/usr/bin/perl
#
# This tools is kind of a hack to be able to maintain the credits list of
# Cabal in a single central location. We then generate the various versions
# of the credits in other places from this source. In particular:
# - The AUTHORS file
# - The gui/credits.h header file
# - The Credits.rtf file used by the Mac OS X port
# - The credits.xml file, part of the DocBook manual
# - Finally, credits.xml, for use on the website (different format than the DocBook one)
#
# Initial version written by Fingolfin in December 2004.
#


use strict;
use Text::Wrap;

if ($Text::Wrap::VERSION < 2001.0929) {
	die "Text::Wrap version >= 2001.0929 is required. You have $Text::Wrap::VERSION\n";
}

my $mode = "";
my $max_name_width;

# Count the level in the section hierarchy, i.e. how deep we are currently nested
# in terms of 'sections'.
my $section_level = 0;

# Count how many sections there have been on this level already
my @section_count = ( 0, 0, 0 );

if ($#ARGV >= 0) {
	$mode = "TEXT" if ($ARGV[0] eq "--text");	# AUTHORS file
	$mode = "XML-WEB" if ($ARGV[0] eq "--xml-website");	# credits.xml (for use on the website)
	$mode = "CPP" if ($ARGV[0] eq "--cpp");		# credits.h (for use by about.cpp)
	$mode = "XML-DOC" if ($ARGV[0] eq "--xml-docbook");		# credits.xml (DocBook)
	$mode = "RTF" if ($ARGV[0] eq "--rtf");		# Credits.rtf (Mac OS X About box)
}

if ($mode eq "") {
	print STDERR "Usage: $0 [--text | --xml-website | --cpp | --xml-docbook | --rtf]\n";
	print STDERR " Just pass --text / --xml-website / --cpp / --xml-docbook / --rtf as parameter, and credits.pl\n";
	print STDERR " will print out the corresponding version of the credits to stdout.\n";
	exit 1;
}

$Text::Wrap::unexpand = 0;
if ($mode eq "TEXT") {
	$Text::Wrap::columns = 78;
	$max_name_width = 28; # The maximal width of a name.
} elsif ($mode eq "CPP") {
	$Text::Wrap::columns = 48;	# Approx.
}

# Convert HTML entities to ASCII for the plain text mode
sub html_entities_to_ascii {
	my $text = shift;

	# For now we hardcode these mappings
	# &aacute;  -> a
	# &eacute;  -> e
	# &iacute;  -> i
	# &igrave;  -> i
	# &oacute;  -> o
	# &oslash;  -> o
	# &ouml;    -> o / oe
	# &auml;    -> a
	# &euml;    -> e
	# &uuml;    -> ue
	# &aring;   -> aa
	# &amp;     -> &
	# &#322;    -> l
	# &#347;    -> s
	# &Scaron;  -> S
	# &ntilde;  -> n
	$text =~ s/&aacute;/a/g;
	$text =~ s/&eacute;/e/g;
	$text =~ s/&iacute;/i/g;
	$text =~ s/&igrave;/i/g;
	$text =~ s/&oacute;/o/g;
	$text =~ s/&oslash;/o/g;
	$text =~ s/&#322;/l/g;
	$text =~ s/&#347;/s/g;
	$text =~ s/&Scaron;/S/g;
	$text =~ s/&aring;/aa/g;
	$text =~ s/&ntilde;/n/g;

	$text =~ s/&auml;/a/g;
	$text =~ s/&euml;/e/g;
	$text =~ s/&uuml;/ue/g;
	# HACK: Torbj*o*rn but G*oe*ffringmann and R*oe*ver and J*oe*rg
	$text =~ s/Torbj&ouml;rn/Torbjorn/g;
	$text =~ s/&ouml;/oe/g;

	$text =~ s/&amp;/&/g;

	return $text;
}

# Convert HTML entities to C++ characters
sub html_entities_to_cpp {
	my $text = shift;

	# The numerical values are octal!
	$text =~ s/&aacute;/\\341/g;
	$text =~ s/&eacute;/\\351/g;
	$text =~ s/&iacute;/\\355/g;
	$text =~ s/&igrave;/\\354/g;
	$text =~ s/&oacute;/\\363/g;
	$text =~ s/&oslash;/\\370/g;
	$text =~ s/&#322;/l/g;
	$text =~ s/&#347;/s/g;
	$text =~ s/&Scaron;/S/g;
	$text =~ s/&aring;/\\345/g;
	$text =~ s/&ntilde;/\\361/g;

	$text =~ s/&auml;/\\344/g;
	$text =~ s/&euml;/\\353/g;
	$text =~ s/&ouml;/\\366/g;
	$text =~ s/&uuml;/\\374/g;

	$text =~ s/&amp;/&/g;

	return $text;
}

# Convert HTML entities to RTF codes
# This is using the Mac OS Roman encoding
sub html_entities_to_rtf {
	my $text = shift;

	$text =~ s/&aacute;/\\'87/g;
	$text =~ s/&eacute;/\\'8e/g;
	$text =~ s/&iacute;/\\'92/g;
	$text =~ s/&igrave;/\\'93/g;
	$text =~ s/&oacute;/\\'97/g;
	$text =~ s/&oslash;/\\'bf/g;
	$text =~ s/&aring;/\\'8c/g;
	# The following numerical values are octal!
	$text =~ s/&#322;/\\uc0\\u322 /g;
	$text =~ s/&Scaron;/\\uc0\\u540 /g;

	# Back to hex numbers
	$text =~ s/&ntilde;/\\'96/g;

	$text =~ s/&auml;/\\'8a/g;
	$text =~ s/&euml;/\\'eb/g;
	$text =~ s/&ouml;/\\'9a/g;
	$text =~ s/&uuml;/\\'9f/g;

	$text =~ s/&amp;/&/g;

	return $text;
}

# Convert HTML entities to TeX codes
sub html_entities_to_tex {
	my $text = shift;

	$text =~ s/&aacute;/\\'a/g;
	$text =~ s/&eacute;/\\'e/g;
	$text =~ s/&iacute;/\\'i/g;
	$text =~ s/&igrave;/\\`\\i/g;
	$text =~ s/&oacute;/\\'o/g;
	$text =~ s/&oslash;/{\\o}/g;
	$text =~ s/&aring;/\\aa /g;
	$text =~ s/&#322;/{\\l}/g;
	$text =~ s/&Scaron;/{\\v S}/g;
	$text =~ s/&ntilde;/\\˜n/g;

	$text =~ s/&auml;/\\"a/g;
	$text =~ s/&ouml;/\\"o/g;
	$text =~ s/&euml;/\\"e/g;
	$text =~ s/&uuml;/\\"u/g;

	$text =~ s/&amp;/\\&/g;

	return $text;
}

#
# Small reference of the RTF commands used here:
#
#  \fs28   switches to 14 point font (28 = 2 * 14)
#  \pard   reset to default paragraph properties
#
#  \ql     left-aligned text
#  \qr     right-aligned text
#  \qc     centered text
#  \qj     justified text
#
#  \b      turn on bold
#  \b0     turn off bold
#
# For more information: <http://latex2rtf.sourceforge.net/rtfspec.html>
#

sub begin_credits {
	my $title = shift;

	if ($mode eq "TEXT") {
		#print html_entities_to_ascii($title)."\n";
	} elsif ($mode eq "RTF") {
		print '{\rtf1\mac\ansicpg10000' . "\n";
		print '{\fonttbl\f0\fswiss\fcharset77 Helvetica-Bold;\f1\fswiss\fcharset77 Helvetica;}' . "\n";
		print '{\colortbl;\red255\green255\blue255;\red0\green128\blue0;\red128\green128\blue128;}' . "\n";
		print '\vieww6920\viewh15480\viewkind0' . "\n";
		print "\n";
	} elsif ($mode eq "CPP") {
		print "// This file was generated by credits.pl. Do not edit by hand!\n";
		print "static const char *credits[] = {\n";
	} elsif ($mode eq "XML-DOC") {
		print "<?xml version='1.0'?>\n";
		print "<!-- This file was generated by credits.pl. Do not edit by hand! -->\n";
		print "<!DOCTYPE appendix PUBLIC '-//OASIS//DTD DocBook XML V4.2//EN'\n";
		print "       'http://www.oasis-open.org/docbook/xml/4.2/docbookx.dtd'>\n";
		print "<appendix id='credits'>\n";
		print "  <title>" . $title . "</title>\n";
		print "  <informaltable frame='none'>\n";
		print "  <tgroup cols='3' align='left' colsep='0' rowsep='0'>\n";
		print "  <colspec colname='start' colwidth='0.5cm'/>\n";
		print "  <colspec colname='name' colwidth='4cm'/>\n";
		print "  <colspec colname='job'/>\n";
		print "  <tbody>\n";
	} elsif ($mode eq "XML-WEB") {
		print "<?xml version='1.0'?>\n";
		print "<!-- This file was generated by credits.pl. Do not edit by hand! -->\n";
		print "<credits>\n";
	}
}

sub end_credits {
	if ($mode eq "TEXT") {
	} elsif ($mode eq "RTF") {
		print "}\n";
	} elsif ($mode eq "CPP") {
		print "};\n";
	} elsif ($mode eq "XML-DOC") {
		print "  </tbody>\n";
		print "  </tgroup>\n";
		print "  </informaltable>\n";
		print "</appendix>\n";
	} elsif ($mode eq "XML-WEB") {
		print "</credits>\n";
	}
}

sub begin_section {
	my $title = shift;

	if ($mode eq "TEXT") {
		$title = html_entities_to_ascii($title);

		if ($section_level >= 2) {
			$title .= ":"
		}

		print "  " x $section_level . $title."\n";
		if ($section_level eq 0) {
			print "  " x $section_level . "*" x (length $title)."\n";
		} elsif ($section_level eq 1) {
			print "  " x $section_level . "-" x (length $title)."\n";
		}
	} elsif ($mode eq "RTF") {
		$title = html_entities_to_rtf($title);

		# Center text
		print '\pard\qc' . "\n";
		print '\f0\b';
		if ($section_level eq 0) {
			print '\fs40 ';
		} elsif ($section_level eq 1) {
			print '\fs32 ';
		}

		# Insert an empty line before this section header, *unless*
		# this is the very first section header in the file.
		if ($section_level > 0 || @section_count[0] > 0) {
			print "\\\n";
		}
		print '\cf2 ' . $title . "\n";
		print '\f1\b0\fs24 \cf0 \\' . "\n";
	} elsif ($mode eq "CPP") {
		if ($section_level eq 0) {
			# TODO: Would be nice to have a 'fat' or 'large' mode for
			# headlines...
			my $ascii_title = html_entities_to_ascii($title);
			$title = html_entities_to_cpp($title);
			if ($ascii_title ne $title) {	
				print '"A1""'.$ascii_title.'",' . "\n";
			}
			print '"C1""'.$title.'",' . "\n";
			print '"",' . "\n";
		} else {
			my $ascii_title = html_entities_to_ascii($title);
			$title = html_entities_to_cpp($title);
			if ($ascii_title ne $title) {	
				print '"A1""'.$ascii_title.'",' . "\n";
			}
			print '"C1""'.$title.'",' . "\n";
		}
	} elsif ($mode eq "XML-DOC") {
		print "  <row><entry namest='start' nameend='job'>";
		print "<emphasis role='bold'>" . $title . ":</emphasis>";
		print "</entry></row>\n";
	} elsif ($mode eq "XML-WEB") {
		if ($section_level eq 0) {
			print "\t<section>\n";
			print "\t\t<title>" . $title . "</title>\n";
		} elsif ($section_level eq 1) {
			print "\t\t<subsection>\n";
			print "\t\t\t<title>" . $title . "</title>\n";
		} else {
			#print "\t\t\t<group>" . $title . "</group>\n";
			#print "\t\t\t\t<name>" . $title . "</name>\n";
		}
	}

	# Implicit start of person list on section level 2
	if ($section_level >= 2) {
		begin_persons($title);
	}
	@section_count[$section_level]++;
	$section_level++;
	@section_count[$section_level] = 0;
}

sub end_section {
	$section_level--;

	# Implicit end of person list on section level 2
	if ($section_level >= 2) {
		end_persons();
	}

	if ($mode eq "TEXT") {
		# nothing
	} elsif ($mode eq "RTF") {
		# nothing
	} elsif ($mode eq "CPP") {
		print '"",' . "\n";
	} elsif ($mode eq "XML-DOC") {
		print "  <row><entry namest='start' nameend='job'> </entry></row>\n\n";
	} elsif ($mode eq "XML-WEB") {
		if ($section_level eq 0) {
			print "\t</section>\n";
		} elsif ($section_level eq 1) {
			print "\t\t</subsection>\n";
		} else {
			#print "\t\t\t</group>\n";
		}
	}
}

sub begin_persons {
	my $title = shift;
	if ($mode eq "XML-WEB") {
		print "\t\t\t<group>\n";
		print "\t\t\t\t<name>" . $title . "</name>\n";
		#print "\t\t\t\t<persons>\n";
	}
}

sub end_persons {
	if ($mode eq "TEXT") {
		print "\n";
	} elsif ($mode eq "RTF") {
		# nothing
	} elsif ($mode eq "XML-WEB") {
		#print "\t\t\t\t</persons>\n";
		print "\t\t\t</group>\n";
	}
}

sub add_person {
	my $name = shift;
	my $nick = shift;
	my $desc = shift;
	my $tab;

	if ($mode eq "TEXT") {
		my $min_name_width = length $desc > 0 ? $max_name_width : 0;
		$name = $nick if $name eq "";
		$name = html_entities_to_ascii($name);
		if (length $name > $max_name_width) {
			print STDERR "Warning: max_name_width is too small (" . $max_name_width . " < " . (length $name) . " for \"" . $name. "\")\n";
		}
		$desc = html_entities_to_ascii($desc);

		$tab = " " x ($section_level * 2 + 1);
		printf $tab."%-".$min_name_width.".".$max_name_width."s", $name;

		# Print desc wrapped
		if (length $desc > 0) {
		  my $inner_indent = ($section_level * 2 + 1) + $max_name_width + 3;
		  my $multitab = " " x $inner_indent;
		  print " - " . substr(wrap($multitab, $multitab, $desc), $inner_indent);
		}
		print "\n";
	} elsif ($mode eq "RTF") {
		$name = $nick if $name eq "";
		$name = html_entities_to_rtf($name);

		# Center text
		print '\pard\qc' . "\n";
		# Activate 1.5 line spacing mode
		print '\sl360\slmult1';
		# The name
		print $name . "\\\n";
		# Description using italics
		if (length $desc > 0) {
			$desc = html_entities_to_rtf($desc);
			print '\pard\qc' . "\n";
			print "\\cf3\\i " . $desc . "\\i0\\cf0\\\n";
		}
	} elsif ($mode eq "CPP") {
		$name = $nick if $name eq "";
		my $ascii_name = html_entities_to_ascii($name);
		$name = html_entities_to_cpp($name);

		if ($ascii_name ne $name) {
			print '"A0""'.$ascii_name.'",' . "\n";
		}
		print '"C0""'.$name.'",' . "\n";

		# Print desc wrapped
		if (length $desc > 0) {
			my $ascii_desc = html_entities_to_ascii($desc);
			$desc = html_entities_to_cpp($desc);
			if ($ascii_desc ne $desc) {	
				print '"A2""'.$ascii_desc.'",' . "\n";
			}
			print '"C2""'.$desc.'",' . "\n";
		}
	} elsif ($mode eq "XML-DOC") {
		$name = $nick if $name eq "";
		print "  <row><entry namest='name'>" . $name . "</entry>";
		print "<entry>" . $desc . "</entry></row>\n";
	} elsif ($mode eq "XML-WEB") {
		$name = "???" if $name eq "";
		print "\t\t\t\t<person>\n";
		print "\t\t\t\t\t<name>" . $name . "</name>\n";
		print "\t\t\t\t\t<alias>" . $nick . "</alias>\n";
		print "\t\t\t\t\t<description>" . $desc . "</description>\n";
		print "\t\t\t\t</person>\n";
	}
}

sub add_paragraph {
	my $text = shift;
	my $tab;

	if ($mode eq "TEXT") {
		$tab = " " x ($section_level * 2 + 1);
		print wrap($tab, $tab, html_entities_to_ascii($text))."\n";
		print "\n";
	} elsif ($mode eq "RTF") {
		$text = html_entities_to_rtf($text);
		# Center text
		print '\pard\qc' . "\n";
		print "\\\n";
		print $text . "\\\n";
	} elsif ($mode eq "CPP") {
		$text = html_entities_to_ascii($text);
		my $line_start = '"C0""';
		my $line_end = '",';
		print $line_start . $text . $line_end . "\n";
		print $line_start . $line_end . "\n";
	} elsif ($mode eq "XML-DOC") {
		print "  <row><entry namest='start' nameend='job'>" . $text . "</entry></row>\n";
		print "  <row><entry namest='start' nameend='job'> </entry></row>\n\n";
	} elsif ($mode eq "XML-WEB") {
		print "\t\t<paragraph>" . $text . "</paragraph>\n";
	}
}

#
# Now follows the actual credits data! The format should be clear, I hope.
# Note that people are sorted by their last name in most cases; in the
# 'Team' section, they are first grouped by category (Engine; porter; misc).
#

begin_credits("Credits");
	begin_section("Cabal Team");
		begin_section("Project Leader");
			begin_persons();
				add_person("Matthew Hoops", "clone2727", "");
			end_persons();
		end_section();
	end_section();


	begin_section("ScummVM Team");
		add_paragraph(
			"For people that have contributed to the ScummVM project, see COPYRIGHT.");
	end_section();


	# HACK!
	$max_name_width = 17;

	begin_section("Special thanks to");
		begin_persons();
			add_person("Daniel Balsom", "DanielFox", "For the original Reinherit (SAGA) code");
			add_person("Sander Buskens", "", "For his work on the initial reversing of Monkey2");
			add_person("", "Canadacow", "For the original MT-32 emulator");
			add_person("Kevin Carnes", "", "For Scumm16, the basis of Cabal's older gfx codecs");
			add_person("Curt Coder", "", "For the original TrollVM (preAGI) code");
			add_person("Patrick Combet", "Dorian Gray", "For the original Gobliiins ADL player");
			add_person("Ivan Dubrov", "", "For contributing the initial version of the Gobliiins engine");
			add_person("DOSBox Team", "", "For their awesome OPL2 and OPL3 emulator");
			add_person("FreeSCI Team", "", "Original SCI engine code");
			add_person("Yusuke Kamiyamane", "", "For contributing some GUI icons");
			add_person("Till Kresslein", "Krest", "For design of modern Cabal GUI");
			add_person("", "Jezar", "For his freeverb filter implementation");
			add_person("Jim Leiterman", "", "Various info on his FM-TOWNS/Marty SCUMM ports");
			add_person("", "lloyd", "For deep tech details about C64 Zak &amp; MM");
			add_person("Sarien Team", "", "Original AGI engine code");
			add_person("Jimmi Th&oslash;gersen", "", "For ScummRev, and much obscure code/documentation");
			add_person("", "Tristan", "For additional work on the original MT-32 emulator");
			add_person("James Woodcock", "", "Soundtrack enhancements");
			add_person("Anton Yartsev", "Zidane", "For the original re-implementation of the Z-Vision engine");
		end_persons();

	add_paragraph(
    "Tony Warriner and everyone at Revolution Software Ltd. for sharing ".
    "with us the source of some of their brilliant games, allowing us to ".
    "release Beneath a Steel Sky as freeware... and generally being ".
    "supportive above and beyond the call of duty.");

	add_paragraph(
    "John Passfield and Steve Stamatiadis for sharing the source of their ".
    "classic title, Flight of the Amazon Queen and also being incredibly ".
    "supportive.");

	add_paragraph(
    "Joe Pearce from The Wyrmkeep Entertainment Co. for sharing the source ".
    "of their famous title Inherit the Earth and always prompt replies to ".
    "our questions.");

	add_paragraph(
    "Aric Wilmunder, Ron Gilbert, David Fox, Vince Lee, and all those at ".
    "LucasFilm/LucasArts who made SCUMM the insane mess to reimplement ".
    "that it is today. Feel free to drop us a line and tell us what you ".
    "think, guys!");

	add_paragraph(
    "Alan Bridgman, Simon Woodroffe and everyone at Adventure Soft for ".
    "sharing the source code of some of their games with us.");

	add_paragraph(
    "John Young, Colin Smythe and especially Terry Pratchett himself for ".
    "sharing the source code of Discworld I &amp; II with us.");

	add_paragraph(
    "Emilio de Paz Arag&oacute;n from Alcachofa Soft for sharing the source code ".
    "of Drascula: The Vampire Strikes Back with us and his generosity with ".
    "freewaring the game.");

	add_paragraph(
    "David P. Gray from Gray Design Associates for sharing the source code ".
    "of the Hugo trilogy.");

	add_paragraph(
    "Broken Sword 2.5 team for providing sources of their engine and their great ".
    "support.");

	add_paragraph(
    "Neil Dodwell and David Dew from Creative Reality for providing the source ".
    "of Dreamweb and for their tremendous support.");

	add_paragraph(
    "Janusz Wi&#347;niewski and Miroslaw Liminowicz from Laboratorium Komputerowe Avalon ".
    "for providing full source code for So&#322;tys and Sfinx and letting us redistribute the games.");

	add_paragraph(
    "Jan Nedoma for providing the sources to the Wintermute-engine, and for his ".
    "support while porting the engine to Cabal.");

	add_paragraph(
    "Bob Bell, Michel Kripalani, and Tommy Yune from Presto Studios for ".
    "providing the source code of The Journeyman Project: Pegasus Prime.");

	add_paragraph(
    "Electronic Arts IP Preservation Team, particularly Stefan Serbicki, and Vasyl Tsvirkunov of ".
    "Electronic Arts for providing the source code of the two Lost Files of Sherlock Holmes games. ".
    "James M. Ferguson and Barry Duncan for their tenacious efforts to recover the sources.");

	add_paragraph(
    "The mindFactory team for writing Broken Sword 2.5, a splendid fan-made sequel, and for sharing ".
    "the source code with us.");

	end_section();

end_credits();
