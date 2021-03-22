int NWP_ellipse2sphere_D(const double *elat, const double *elon,
    const int size, const double slat, const double slon,
    double *lat, double *lon);
int NWP_sphere2ellipse_D(const double *lat, const double *lon,
    const int size, const double slat, const double slon,
    double *elat, double *elon);
int NWP_sphere2oblique_D(const double *lat, const double *lon,
    const int size, const double slat, const double slon,
    double *olat, double *olon);
int NWP_oblique2sphere_D(const double *olat, const double *olon,
    const int size, const double slat, const double slon,
    double *lat, double *lon);
int NWP_sphere2lambert_D(const double *lat, const double *lon,
    const int size, const double slat1, const double slat2,
    const double slon, const double rlat, const double rlon,
    const double rx, const double ry, const double d,
    double *x, double *y);
int NWP_lambert2sphere_D(const double *x, const double *y,
    const int size, const double slat1, const double slat2,
    const double slon, const double rlat, const double rlon,
    const double rx, const double ry, const double d,
    double *lat, double *lon);
int NWP_sphere2mercator_D(const double *lat, const double *lon,
    const int size, const double slat, const double rlat,
    const double rlon, const double rx, const double ry,
    const double d, double *x, double *y);
int NWP_sphere2mercator2_D(const double *lat, const double *lon,
    const int size, const double slat, const double rlat,
    const double rlon, const double rx, const double ry,
    const double d, double *x, double *y);
int NWP_mercator2sphere_D(const double *x, const double *y,
    const int size, const double slat, const double rlat,
    const double rlon, const double rx, const double ry,
    const double d, double *lat, double *lon);
int NWP_sphere2polar_D(const double *lat, const double *lon,
    const int size, const double slat, const double slon,
    const double rlat, const double rlon,
    const double rx, const double ry, const double d,
    double *x, double *y);
int NWP_polar2sphere_D(const double *x, const double *y,
    const int size, const double slat, const double slon,
    const double rlat, const double rlon,
    const double rx, const double ry, const double d,
    double *lat, double *lon);
int NWP_mf_lambert_D(const double *lat, const int size,
    const double slat1, const double slat2, double *mf);
int NWP_mf_mercator_D(const double *lat, const int size,
    const double slat, double *mf);
int NWP_mf_polar_D(const double *lat, const int size,
    const double slat, double *mf);
double NWP_sphere_distance_D(const double alat, const double alon,
    const double blat, const double blon);

int NWP_ellipse2sphere_F(const float *elat, const float *elon,
    const int size, const float slat, const float slon,
    float *lat, float *lon);
int NWP_sphere2ellipse_F(const float *lat, const float *lon,
    const int size, const float slat, const float slon,
    float *elat, float *elon);
int NWP_sphere2oblique_F(const float *lat, const float *lon,
    const int size, const float slat, const float slon,
    float *olat, float *olon);
int NWP_oblique2sphere_F(const float *olat, const float *olon,
    const int size, const float slat, const float slon,
    float *lat, float *lon);
int NWP_sphere2lambert_F(const float *lat, const float *lon,
    const int size, const float slat1, const float slat2,
    const float slon, const float rlat, const float rlon,
    const float rx, const float ry, const float d,
    float *x, float *y);
int NWP_lambert2sphere_F(const float *x, const float *y,
    const int size, const float slat1, const float slat2,
    const float slon, const float rlat, const float rlon,
    const float rx, const float ry, const float d,
    float *lat, float *lon);
int NWP_sphere2mercator_F(const float *lat, const float *lon,
    const int size, const float slat, const float rlat,
    const float rlon, const float rx, const float ry,
    const float d, float *x, float *y);
int NWP_sphere2mercator2_F(const float *lat, const float *lon,
    const int size, const float slat, const float rlat,
    const float rlon, const float rx, const float ry,
    const float d, float *x, float *y);
int NWP_mercator2sphere_F(const float *x, const float *y,
    const int size, const float slat, const float rlat,
    const float rlon, const float rx, const float ry,
    const float d, float *lat, float *lon);
int NWP_sphere2polar_F(const float *lat, const float *lon,
    const int size, const float slat, const float slon,
    const float rlat, const float rlon,
    const float rx, const float ry, const float d,
    float *x, float *y);
int NWP_polar2sphere_F(const float *x, const float *y,
    const int size, const float slat, const float slon,
    const float rlat, const float rlon,
    const float rx, const float ry, const float d,
    float *lat, float *lon);
int NWP_mf_lambert_F(const float *lat, const int size,
    const float slat1, const float slat2, float *mf);
int NWP_mf_mercator_F(const float *lat, const int size,
    const float slat, float *mf);
int NWP_mf_polar_F(const float *lat, const int size,
    const float slat, float *mf);
float NWP_sphere_distance_F(const float alat, const float alon,
    const float blat, const float blon);

