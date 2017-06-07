<?php
    define("MAX_ENTRIES", "1440");
    define("FONT", 2);
    define("XABS", 60);
    define("XDIFF", 360);
    define("YABS", 19);
    define("YDIFF", 270);
    define("ILLEGAL_TEMP", 1000.0);
    define("NOC", 4);                                                           // number of cells to build average on

    function get_data( &$string, &$array, &$min, &$max, &$x_of_max, $trans )
        {
        $file = fopen($string, "r");
        if( !$file )
            exit;

        for( $i=0; $i<XDIFF; $i++ )
            $array[$i] = ILLEGAL_TEMP;
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
                $a[$k] = $parts[12];
                if( $j != 0 )
                    {
                    if( ( (float)$a[$k] < ((float)$t_old - 15.0) ) || ( (float)$a[$k] > ((float)$t_old + 15.0) ) )
                        continue;
                    else
                        $t_old = $a[$k];
                    }
                else
                    $t_old = $a[$k];
                if( (float)$a[$k] > 75.0 )
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

    function get_y( $temp, $min_temp, $tempdiff )
        {
        if( (float)$min_temp >= 0.0 )
            return(YDIFF + YABS - (YDIFF * $temp) / $tempdiff);
        else
            return(YDIFF + YABS - (YDIFF * ($temp-$min_temp)) / $tempdiff);
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
    $min_temp1 = 100.0;
    $max_temp1 = -100.0;
    get_data($string, $temp_1, $min_temp1, $max_temp1, $max_temp1_x, $trans);

    $string = date("Y_m_d", $todaystamp);
    $string = $domain . $string . "data.log";
    $min_temp2 = 100.0;
    $max_temp2 = -100.0;
    get_data($string, $temp_2, $min_temp2, $max_temp2, $max_temp2_x, $trans);

    $im = @ImageCreate (XDIFF+80, YDIFF+80)
        or die ("Kann keinen neuen GD-Bild-Stream erzeugen");
    $background_color = ImageColorAllocate($im, 247, 247, 247);
    $black = ImageColorAllocate($im, 0, 0, 0);
    $grey = ImageColorAllocate($im, 192, 192, 192);
    $blue = ImageColorAllocate($im, 0, 0, 255);
    $red = ImageColorAllocate($im, 255, 0, 0);

    $string = "gefühlte Temperatur";
    $x = (XDIFF + 80 - (strlen($string) * ImageFontWidth(FONT))) / 2;
    ImageString($im, FONT, $x, 2, $string, $black);

    $string = date("j.m.Y G:i ", $todaystamp);
    $string = "Aktuelle Ablesung : " . $string;
    ImageString($im, FONT, XABS-(8*ImageFontWidth(FONT)), YABS+YDIFF+4+ImageFontHeight(FONT)+2, $string, $black);

    $x = XABS + 33 * ImageFontWidth(FONT);
    $tempdiff1 = $max_temp1 - $min_temp1;
    $string = date("d.m.Y", $yesterdaystamp);
    $string .= sprintf("   Minimum : %5.1f°C Maximum : %5.1f°C", $min_temp1, $max_temp1);
    ImageString($im, FONT, XABS-(8*ImageFontWidth(FONT)), YABS+YDIFF+4+2*(ImageFontHeight(FONT)+2), $string, $blue);
    $tempdiff2 = $max_temp2 - $min_temp2;
    $string = date("d.m.Y", $todaystamp);
    $string .= sprintf("   Minimum : %5.1f°C Maximum : %5.1f°C", $min_temp2, $max_temp2);
    ImageString($im, FONT, XABS-(8*ImageFontWidth(FONT)), YABS+YDIFF+4+3*(ImageFontHeight(FONT)+2), $string, $red);

    $x = XABS + 35 * ImageFontWidth(FONT);
    ImageString($im, FONT, $x, YABS+YDIFF+4+ImageFontHeight(FONT)+2, "(C) www.ur9.de", $black);

    if( (float)$min_temp2 < (float)$min_temp1 )
        $min_temp = $min_temp2;
    else
        $min_temp = $min_temp1;
    if( (float)$max_temp2 > (float)$max_temp1 )
        $max_temp = $max_temp2;
    else
        $max_temp = $max_temp1;

    if( (float)$min_temp > 0.0 )
        $min_temp = 0.0;
    if( (float)$max_temp < 0.0 )
        $max_temp = 0.0;

    $tempdiff = $max_temp - $min_temp;
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
    for( $l=(int)$min_temp; $l<=(int)$max_temp; $l++ )
        {
        $y = get_y($l, $min_temp, $tempdiff);;
        ImageLine($im, XABS-5, $y, XABS, $y, $black);
        if( ($l != 0) && (($l % 5) == 0) )
            ImageLine($im, XABS+1, $y, XABS+XDIFF, $y, $grey);
        if( ($l % 2) == 0 )
            {
            $string = sprintf("%d°C", $l);
            $px = XABS - (strlen($string) * ImageFontWidth(FONT)) - 6;
            ImageString($im, FONT, $px, $y-(ImageFontHeight(FONT)/2), $string, $black);
            }
        }
//    if( (float)$min_temp < 0.0 )
//        {
//        $y = YDIFF + YABS - (YDIFF * (0.0 - $min_temp)) / $tempdiff;
//        ImageLine($im, XABS+1, $y, XABS+XDIFF, $y, $grey);
//        }

    show($temp_1, $min_temp, $tempdiff, ILLEGAL_TEMP, $im, $blue);
    show($temp_2, $min_temp, $tempdiff, ILLEGAL_TEMP, $im, $red);
    ImageGIF($im);
    imagedestroy ($im);
?>

