<?php
    define("MAX_ENTRIES", "1440");
    define("FONT", 2);
    define("XABS", 60);
    define("XDIFF", 288);
    define("YABS", 20);
    define("YDIFF", 208);
    define("ILLEGAL_DIR", 1000.0);
    define("NOC", 5);                                                           // number of cells to build average on

    function get_data( &$string, &$array, $trans )
        {
        $file = fopen($string, "r");
        if( !$file )
            exit;

        for( $i=0; $i<XDIFF; $i++ )
            $array[$i] = ILLEGAL_DIR;
        for( $i=0; $i<NOC; $i++ )
            $a[$i] = 0;

        $j = 0;
        $i_old = 0;
        $k = 0;
        while( (!feof($file)) && ($j < MAX_ENTRIES+2) )
            {
            $line = fgets($file, 1024);
            $l = strlen($line);
            if( $l != 0 )
                {
                $line1 = strtr($line, $trans);
                $parts = explode(" ", $line1);
                $times = explode(":", $parts[0]);
                $i = (int)(60 * $times[0] + $times[1]) / NOC;
                if( (int)$i_old != (int)$i )
                    $k = 0;
                $a[$k] = $parts[5];
                if( (float)$parts[8] > 175.0 )
                    continue;
                $k++;
                $array[$i] = 0.0;
                for( $m=0; $m<$k; $m++ )                                        // sum it up
                    $array[$i] = $array[$i] + $a[$m];
                $array[$i] = $array[$i] / $k;                                   // build average
                $i_old = $i;
                }
            $j++;
            }
        fclose($file);
        }

    function get_y( $direction, $min_direction, $directiondiff )
        {
        if( (float)$min_direction >= 0.0 )
            return(YDIFF + YABS - (YDIFF * $direction) / $directiondiff);
        else
            return(YDIFF + YABS - (YDIFF * ($direction-$min_direction)) / $directiondiff);
        }

    function show( $array, $min, $diff, $illegal, $image, $color )
        {
        $y0 = get_y($array[0], $min, $diff);
        for( $j=0; $j<XDIFF-1; $j++ )
            {
            $x = $j + XABS;
            $y1 = get_y($array[$j+1], $min, $diff);
            if( ((float)$array[$j] != $illegal) && ($array[$j+1] != $illegal) )
                ImageLine($image, $x, $y0, $x+1, $y1, $color);
            else if( ((float)$array[$j] == $illegal) && ((float)$array[$j+1] != $illegal) )
                ImageSetPixel($image, $x+1, $y1, $color);
            else if( ((float)$array[$j] != $illegal) && ((float)$array[$j+1] == $illegal) )
                ImageSetPixel($image, $x, $y0, $color);
            $y0 = $y1;                                                          // set next start of line to end of current line
            }
        }

    $domain = "";
    header ("Content-type: image/gif");
    $time = time();
    $todaystamp = $time;
    $yesterdaystamp = $todaystamp - 86400;

    $trans = array("    " => " ", "   " => " ", "  " => " ");

    $string = date("Y_m_d", $yesterdaystamp);
    $string = $domain . $string . "data.log";
    get_data($string, $direction_1, $trans);

    $string = date("Y_m_d", $todaystamp);
    $string = $domain . $string . "data.log";
    get_data($string, $direction_2, $trans);

    $im = @ImageCreate (XDIFF+80, YDIFF+80)
        or die ("Kann keinen neuen GD-Bild-Stream erzeugen");
    $background_color = ImageColorAllocate($im, 247, 247, 247);
    $black = ImageColorAllocate($im, 0, 0, 0);
    $grey = ImageColorAllocate($im, 192, 192, 192);
    $blue = ImageColorAllocate($im, 0, 0, 255);
    $red = ImageColorAllocate($im, 255, 0, 0);

    $string = "Windrichtung";
    $x = (XDIFF + 80 - (strlen($string) * ImageFontWidth(FONT))) / 2;
    ImageString($im, FONT, $x, 2, $string, $black);

    $string = date("j.m.Y G:i ", $todaystamp);
    $string = "Aktuelle Ablesung : " . $string;
    ImageString($im, FONT, XABS-(8*ImageFontWidth(FONT)), YABS+YDIFF+6+ImageFontHeight(FONT)+3, $string, $black);

    $x = XABS + 35 * ImageFontWidth(FONT);
    ImageString($im, FONT, $x, YABS+YDIFF+6+ImageFontHeight(FONT)+3, "(C) www.ur9.de", $black);

    $string = date("d.m.Y", $yesterdaystamp);
    ImageString($im, FONT, XABS-(8*ImageFontWidth(FONT)), YABS+YDIFF+6+2*(ImageFontHeight(FONT)+3), $string, $blue);
    $x = XABS + 4 * ImageFontWidth(FONT);
    $string = date("d.m.Y", $todaystamp);
    ImageString($im, FONT, $x, YABS+YDIFF+6+2*(ImageFontHeight(FONT)+3), $string, $red);

    $min_direction = 0.0;
    $max_direction = 359.9;

    $directiondiff = $max_direction - $min_direction;
    ImageLine($im, XABS, YABS, XABS, YABS+YDIFF, $black);
    ImageLine($im, XABS, YABS+YDIFF, XABS+XDIFF, YABS+YDIFF, $black);
    ImageString($im, FONT, XABS-(8*ImageFontWidth(FONT)), YABS+YDIFF+6, "Zeit:", $black);
    for( $l=0; $l<=12; $l++ )
        {
        $xmarker = (XDIFF / 12) * $l + XABS;
        ImageLine($im, $xmarker, YABS+YDIFF, $xmarker, YABS+YDIFF+5, $black);
        if( ($l != 0) && (($l % 4) == 0) )
            ImageDashedLine($im, $xmarker, YABS+YDIFF-1, $xmarker, YABS, $grey);
        if( ($l % 4) == 0 )
            {
            $string = sprintf("%d:00", $l * 2);
            $px = $xmarker - (strlen($string) * ImageFontWidth(FONT) / 2);
            ImageString($im, FONT, $px, YABS+YDIFF+6, $string, $black);
            }
        }
    for( $l=(int)$min_direction; $l<=(int)$max_direction+1; $l++ )
        {
        if( ($l % 30) == 0 )
            {
            $y = get_y($l, $min_direction, $directiondiff);;
            ImageLine($im, XABS-5, $y, XABS, $y, $black);
            }
        if( ($l % 90) == 0 )
            {
            if( ($l == 0) || ($l == 360) )
                $string = "N";
            if( $l == 90 )
                $string = "O";
            if( $l == 180 )
                $string = "S";
            if( $l == 270 )
                $string = "W";
            $string .= sprintf("% 4d°", $l);
            $px = XABS - (strlen($string) * ImageFontWidth(FONT)) - 8;
            ImageString($im, FONT, $px, $y-(ImageFontHeight(FONT)/2), $string, $black);
            }
        }
    show($direction_1, $min_direction, $directiondiff, ILLEGAL_DIR, $im, $blue);
    show($direction_2, $min_direction, $directiondiff, ILLEGAL_DIR, $im, $red);
    ImageGIF($im);
    imagedestroy ($im);
?>

