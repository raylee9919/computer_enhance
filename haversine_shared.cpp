/* ========================================================================

   (C) Copyright 2025 by Sung Woo Lee, All Rights Reserved.
   
   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.
   
   ======================================================================== */


static double
square(double x) 
{
    return x * x; 
}

static double
radians_from_degress(double degrees)
{
    double result = 0.01745329251994329577 * degrees;
    return result;
}

static double
haversine(double x0, double y0, double x1, double y1, double earth_radius = 6372.8)
{
    double lat1 = y0;
    double lat2 = y1;
    double lon1 = x0;
    double lon2 = x1;
    
    double dLat = radians_from_degress(lat2 - lat1);
    double dLon = radians_from_degress(lon2 - lon1);
    lat1 = radians_from_degress(lat1);
    lat2 = radians_from_degress(lat2);
    
    double a = square(sin(dLat/2.0)) + cos(lat1)*cos(lat2)*square(sin(dLon/2));
    double c = 2.0 * asin(sqrt(a));
    
    double result = earth_radius * c;
    
    return result;
}

const char *haversine_json_filename = "haversine_input.json";
const char *haversine_answer_filename = "haversine_answer.f64";
