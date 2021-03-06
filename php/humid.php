<?php
    define("MAX_ENTRIES", "1440");
    define("FONT", 2);
    define("XABS", 60);
    define("XDIFF", 360);
    define("YABS", 19);
    define("YDIFF", 270);
    define("ILLEGAL_HUMIDITY", 2000.0);
    define("NOC", 4);                                                           // number of cells to build average on

    function get_data( &$string, &$array, &$min, &$max, &$x_of_max, $trans )
        {
        $file = fopen($string, "r");
        if( !$file )
            exit;

        for( $i=0; $i<XDIFF; $i++ )
            $array[$i] = ILLEGAL_HUMIDITY;
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
//                if( ($j == 0) && ($i != 0) )
//                    continue;
                if( (int)$i_old != (int)$i )
                    $k = 0;
                $a[$k] = $parts[4];
//                if( (float)$a[$k] < 0.5 )                                     // smaller than 0.5% is no realistic value
//                    continue;                                                 // so skip it !
                if( (float)$a[$k] > 100.1 )                                     // smaller than 800hPa is no realistic value
                    continue;                                                   // so skip it !
                if( (float)$a[$k] < (float)$min )
                    $min = $a[$k];
                if( (float)$a[$k] > (float)$max )
                    {
                    $max = $a[$k];
                    $x_of_max = $i;
                    }
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

    function get_y( $humidity, $min_humidity, $humiditydiff )
        {
        return((float)YDIFF + (float)YABS - (float)((float)YDIFF * (float)($humidity-$min_humidity)) / (float)$humiditydiff);
        }

    function show( $array, $min, $diff, $illegal, $image, $color )
        {
        $y0 = get_y($array[0], $min, $diff);
        for( $j=0; $j<XDIFF-1; $j++ )
            {
            $x = $j + XABS;
            $y1 = get_y($array[$j+1], $min, $diff);
            if( ((float)$array[$j] != (float)$illegal) && ($array[$j+1] != (float)$illegal) )
                ImageLine($image, $x, $y0, $x+1, $y1, $color);
            else if( ((float)$array[$j] == (float)$illegal) && ((float)$array[$j+1] != (float)$illegal) )
                ImageSetPixel($image, $x+1, $y1, $color);
            else if( ((float)$array[$j] != (float)$illegal) && ((float)$array[$j+1] == (float)$illegal) )
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
    $min_humidity1 = 200.0;
    $max_humidity1 = -1.0;
    get_data($string, $humidity_1, $min_humidity1, $max_humidity1, $max_humidity1_x, $trans);

    $string = date("Y_m_d", $todaystamp);
    $string = $domain . $string . "data.log";
    $min_humidity2 = 200.0;
    $max_humidity2 = -1.0;
    get_data($string, $humidity_2, $min_humidity2, $max_humidity2, $max_humidity2_x, $trans);

    $im = @ImageCreate (XDIFF+80, YDIFF+80)
        or die ("Kann keinen neuen GD-Bild-Stream erzeugen");
    $background_color = ImageColorAllocate($im, 247, 247, 247);
    $black = ImageColorAllocate($im, 0, 0, 0);
    $grey = ImageColorAllocate($im, 192, 192, 192);
    $blue = ImageColorAllocate($im, 0, 0, 255);
    $red = ImageColorAllocate($im, 255, 0, 0);

    $string = "relative Luftfeuchtigkeit";
    $x = (XDIFF + 80 - (strlen($string) * ImageFontWidth(FONT))) / 2;
    ImageString($im, FONT, $x, 2, $string, $black);

    $string = date("j.m.Y G:i ", $todaystamp);
    $string = "Aktuelle Ablesung : " . $string;
    ImageString($im, FONT, XABS-(8*ImageFontWidth(FONT)), YABS+YDIFF+4+ImageFontHeight(FONT)+2, $string, $black);

    $string = date("d.m.Y", $yesterdaystamp);
    $string .= sprintf("  Minimum : %5.1f%%  Maximum : %5.1f%%", $min_humidity1, $max_humidity1);
    ImageString($im, FONT, XABS-(8*ImageFontWidth(FONT)), YABS+YDIFF+4+2*(ImageFontHeight(FONT)+2), $string, $blue);
    $humiditydiff2 = $max_humidity2 - $min_humidity2;
    $string = date("d.m.Y", $todaystamp);
    $string .= sprintf("  Minimum : %5.1f%%  Maximum : %5.1f%%", $min_humidity2, $max_humidity2);
    ImageString($im, FONT, XABS-(8*ImageFontWidth(FONT)), YABS+YDIFF+4+3*(ImageFontHeight(FONT)+2), $string, $red);

    $x = XABS + 35 * ImageFontWidth(FONT);
    ImageString($im, FONT, $x, YABS+YDIFF+4+ImageFontHeight(FONT)+2, "(C) www.ur9.de", $black);

    $min_humidity = 0.0;
    $max_humidity = 100.0;
    $min_humidity = (int)$min_humidity;
    $max_humidity = (int)$max_humidity;
    $humiditydiff = $max_humidity - $min_humidity;

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
    for( $l=(int)$min_humidity; $l<=(int)$max_humidity; $l++ )
        {
        if( ($l % 10) == 0 )
            {
            $y = get_y($l, $min_humidity, $humiditydiff);;
            ImageLine($im, XABS-5, $y, XABS, $y, $black);
            $string = sprintf("%d%%", $l);
            $px = XABS - (strlen($string) * ImageFontWidth(FONT)) - 8;
            ImageString($im, FONT, $px, $y-(ImageFontHeight(FONT)/2), $string, $black);
            }
        }

    show($humidity_1, $min_humidity, $humiditydiff, ILLEGAL_HUMIDITY, $im, $blue);
    show($humidity_2, $min_humidity, $humiditydiff, ILLEGAL_HUMIDITY, $im, $red);

    ImageGIF($im);
    imagedestroy ($im);
?>

