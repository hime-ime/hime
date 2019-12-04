#
# Copyright (C) 2011 Lu, Chao-Ming (Tetralet).  All rights reserved.
# 
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
# 
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
# 
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
#

#!/bin/sh

export LC_ALL=zh_TW.UTF8

CONVERT=`whereis -b convert | tr -d '\n' | sed 's/^convert: *//g'`
if [ -z "$CONVERT" ]; then
	echo "Command 'convert' is not find. Please install imagemagick package and try again!"
	exit 0
fi

print_help()
{
	case $1 in
		-h|--help)
			echo "Usage: $0 SourceImage.png HalfCharImage.png FullCharImage.png Font_Patch Font_Color Mini_Color"
			echo "Example: $0 HIME.png HIME_Half.png HIME_Full.png ~/.fonts/DroidSansFallback.ttf \"#1650b8\" \"c81bca\""
			exit 0
			;;
	esac
}

if [ "$1" == '' ]; then
	print_help -h
fi
if [ ! -f "$1" ]; then
	echo "ERROR: Source Image file $1 Not find!"
	echo ""
	print_help -h
fi
SOURCE_IMAGE="$1"

if [ "$2" == '' ]; then
	print_help -h
fi
if [ ! -f "$2" ]; then
	echo "ERROR: HalfChar Image file $1 Not find!"
	echo ""
	print_help -h
fi
HALF_IMAGE="$2"

if [ "$3" == '' ]; then
	print_help -h
fi
if [ ! -f "$3" ]; then
	echo "ERROR: FullChar Image file $1 Not find!"
	echo ""
	print_help -h
fi
FULL_IMAGE="$3"

if [ "$4" == '' ]; then
	print_help -h
fi
if [ ! -f "$4" ]; then
	echo "ERROR: Font file $4 Not find!"
	echo ""
	print_help -h
fi
FONT_FILE="$4"

if [ "$5" == '' ]; then
	print_help -h
fi
COLOR="$5"

if [ "$6" == '' ]; then
	print_help -h
fi
MINI_COLOR="$6"

convert_word()
{
	while [ "$1" != '' ]; do
		FILE=$1
		shift
		if [ "$1" != '' ]; then
			WORD=$1
		else
			echo "ERROR: No target file inputed!"
			exit 1
		fi
		shift

		SIZE=`echo "$WORD" | wc -m`
		if [ "$WORD" == "En" ]; then
			SIZE=2
		fi
		WORK_IMAGE="$SOURCE_IMAGE"
		echo "Trying to draw '$WORD' (Size $SIZE) on $SOURCE_IMAGE to $FILE.png ..."
		case $SIZE in
			1)
				FONT_SIZE=18
				DRAW_STR="text +0-1 '$WORD'"
				TEMP_FILE=""
			;;
			2)
				FONT_SIZE=18
				DRAW_STR="text +0-1 '$WORD'"
				TEMP_FILE=""
			;;
			3|4)
				if [ "$FILE" == "half-simp" -o "$FILE" == "half-trad" ]; then
					WORK_IMAGE="$HALF_IMAGE"
				fi
				if [ "$FILE" == "full-simp" -o "$FILE" == "full-trad" ]; then
					WORK_IMAGE="$FULL_IMAGE"
				fi
				ORIGINAL_WORD="$WORD"
				WORD=`echo $ORIGINAL_WORD | sed -e 's/\(.\)\(.\)/\1/g'`
				FONT_SIZE=14
				DRAW_STR="text -4-4 '$WORD'"
				TEMP_FILE="hime_temp"
				convert -pointsize $FONT_SIZE \
					-font $FONT_FILE \
					-fill $COLOR \
					-gravity center \
					-draw "$DRAW_STR" \
					$WORK_IMAGE "$TEMP_FILE.png"
				WORD=`echo $ORIGINAL_WORD | sed -e 's/\(.\)\(.\)/\2/g'`
				FONT_SIZE=11
				DRAW_STR="text +5+5 '$WORD'"
			;;
			*)
				echo "The input word '$WORD' is too long!"
				exit 1
			;;
		esac
		if [ "$TEMP_FILE" == '' -o ! -f "$TEMP_FILE.png" ]; then
				convert -pointsize $FONT_SIZE \
				-font $FONT_FILE \
				-fill $COLOR \
				-gravity center \
				-draw "$DRAW_STR" \
				$WORK_IMAGE "$FILE.png"
		else
			convert -pointsize $FONT_SIZE \
				-font $FONT_FILE \
				-fill $MINI_COLOR \
				-gravity center \
				-draw "$DRAW_STR" \
				"$TEMP_FILE.png" "$FILE.png"
			rm "$TEMP_FILE.png"
		fi
	done
}

convert_word 'hime-tray' 'En' \
	     '4corner' '四' \
	     'NewCJ3' '亂' \
	     'amis' '阿' \
	     'ar30' '行' \
	     'chewing' '酷' \
	     'cj' '倉' \
	     'dayi3' '易' \
	     'erbi' '二' \
	     'ez' '輕' \
 	     'fcitx-qxm' '冰' \
	     'fcitx-wanfeng' '晚' \
	     'greek' 'αβ' \
	     'esperanto' '★' \
	     'hakka' '客' \
	     'halfwidth-kana' 'ア' \
	     'intcode' '內' \
	     'jtcj_gb' '仓' \
	     'juyin' '注' \
	     'jyutping' '粵' \
	     'kana-nippon' 'あ' \
	     'latin-letters' 'ā' \
	     'newcj' '新' \
	     'noseeing' '嘸' \
	     'paiwan' '排' \
	     'pinyin' '拼' \
	     'sakura' '櫻' \
	     'scj' '快' \
	     'shuangpin' '双' \
	     'simplex' '簡' \
	     'symbols' '%' \
	     'taiwan' '台' \
	     'telecode' '電' \
	     'tsin' '詞' \
	     'tsou' '鄒' \
	     'wm2' '象' \
	     'wubi' '五' \
	     'russian' 'Э' \
	     'hangul' '한' \
	     'cj5' '倉五' \
	     'cj-punc' '倉；' \
	     'en-kana-nippon' 'あＥ' \
	     'en-tsin' '詞Ｅ' \
	     'half-simp' '　简' \
	     'half-trad' '　 ' \
	     'full-simp' '　简' \
	     'full-trad' '　　' \
	     'gdayi3' '易三'
