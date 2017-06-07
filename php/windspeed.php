<?php
    define("MAX_ENTRIES", "1440");
    define("FONT", 2);
    define("XABS", 60);
    define("XDIFF", 288);
    define("YABS", 19);
    define("YDIFF", 208);
    define("ILLEGAL_SPEED", 1000.0);
    define("NOC", 5);                                                           // number of cells to build average on

    function get_data( &$string, &$array, &$min, &$max, &$x_of_max, $trans )
        {
        $file = fopen($string, "r");
        if( !$file )
            exit;

        for( $i=0; $i<XDIFF; $i++ )
            $array[$i] = ILLEGAL_SPEED;
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
                $a[$k] = $parts[8];
                if( (float)$a[$k] > 175.0 )
                    continue;
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

    function get_y( $speed, $min_speed, $speeddiff )
        {
        if( (float)$min_speed >= 0.0 )
            return(YDIFF + YABS - (YDIFF * $speed) / $speeddiff);
        else
            return(YDIFF + YABS - (YDIFF * ($speed-$min_speed)) / $speeddiff);
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

    function cross( $x, $y, $diff, $image, $color )
        {
        $x = $x - ($diff / 2) + XABS;
        $y = $y - ($diff / 2);
        ImageLine($image, $x, $y, $x+$diff, $y+$diff, $color);
        ImageLine($image, $x, $y+$diff, $x+$diff, $y, $color);
        }

    $domain = "";
    header ("Content-type: image/gif");
    $time = time();
    $todaystamp = $time;
    $yesterdaystamp = $todaystamp - 86400;

    $trans = array("    " => " ", "   " => " ", "  " => " ");

    $string = date("Y_m_d", $yesterdaystamp);
    $string = $domain . $string . "data.log";
    $min_speed1 = 100.0;
    $max_speed1 = -100.0;
    get_data($string, $speed_1, $min_speed1, $max_speed1, $max_speed1_x, $trans);

    $string = date("Y_m_d", $todaystamp);
    $string = $domain . $string . "data.log";
    $min_speed2 = 100.0;
    $max_speed2 = -100.0;
    get_data($string, $speed_2, $min_speed2, $max_speed2, $max_speed2_x, $trans);

    $im = @ImageCreate (XDIFF+80, YDIFF+80)
        or die ("Kann keinen neuen GD-Bild-Stream erzeugen");
    $background_color = ImageColorAllocate($im, 247, 247, 247);
    $black = ImageColorAllocate($im, 0, 0, 0);
    $grey = ImageColorAllocate($im, 192, 192, 192);
    $blue = ImageColorAllocate($im, 0, 0, 255);
    $red = ImageColorAllocate($im, 255, 0, 0);

    $string = "Windgeschwindigkeit";
    $x = (XDIFF + 80 - (strlen($string) * ImageFontWidth(FONT))) / 2;
    ImageString($im, FONT, $x, 2, $string, $black);

    $string = date("j.m.Y G:i ", $todaystamp);
    $string = "Aktuelle Ablesung : " . $string;
    ImageString($im, FONT, XABS-(8*ImageFontWidth(FONT)), YABS+YDIFF+4+ImageFontHeight(FONT)+2, $string, $black);

    $speeddiff1 = $max_speed1 - $min_speed1;
    $string = date("d.m.Y", $yesterdaystamp);
    $string .= sprintf("  Minimum : %5.1fkm/h Maximum : %5.1fkm/h", $min_speed1, $max_speed1);
    ImageString($im, FONT, XABS-(8*ImageFontWidth(FONT)), YABS+YDIFF+4+2*(ImageFontHeight(FONT)+2), $string, $blue);
    $speeddiff2 = $max_speed2 - $min_speed2;
    $string = date("d.m.Y", $todaystamp);
    $string .= sprintf("  Minimum : %5.1fkm/h Maximum : %5.1fkm/h", $min_speed2, $max_speed2);
    ImageString($im, FONT, XABS-(8*ImageFontWidth(FONT)), YABS+YDIFF+4+3*(ImageFontHeight(FONT)+2), $string, $red);

    $x = XABS + 35 * ImageFontWidth(FONT);
    ImageString($im, FONT, $x, YABS+YDIFF+4+ImageFontHeight(FONT)+2, "(C) www.ur9.de", $black);

    if( (float)$min_speed2 < (float)$min_speed1 )
        $min_speed = $min_speed2;
    else
        $min_speed = $min_speed1;
    if( (float)$max_speed2 > (float)$max_speed1 )
        $max_speed = $max_speed2;
    else
        $max_speed = $max_speed1;

    if( (float)$min_speed > 0.0 )
        $min_speed = 0.0;

    if( (float)$max_speed < 5.0 )
        $max_speed = 5.1;

    $speeddiff = $max_speed - $min_speed;
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
    for( $l=(int)$min_speed; $l<=(int)$max_speed; $l++ )
        {
        $y = get_y($l, $min_speed, $speeddiff);;
        ImageLine($im, XABS-5, $y, XABS, $y, $black);
        if( ($l % 5) == 0 )
            {
            $string = sprintf("%dkm/h", $l);
            $px = XABS - (strlen($string) * ImageFontWidth(FONT)) - 8;
            ImageString($im, FONT, $px, $y-(ImageFontHeight(FONT)/2), $string, $black);
            }
        }

    show($speed_1, $min_speed, $speeddiff, ILLEGAL_SPEED, $im, $blue);
    show($speed_2, $min_speed, $speeddiff, ILLEGAL_SPEED, $im, $red);

    $y = get_y($max_speed1, $min_speed, $speeddiff);
    cross($max_speed1_x, $y, 4, $im, $blue);
    $y = get_y($max_speed2, $min_speed, $speeddiff);
    cross($max_speed2_x, $y, 4, $im, $red);

    ImageGIF($im);
    imagedestroy ($im);
?>

