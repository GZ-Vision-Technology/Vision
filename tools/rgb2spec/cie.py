# -*- coding:utf-8 -*-

import numpy as np

CIE_LAMBDA_MIN = 360.0
CIE_LAMBDA_MAX = 830.0
CIE_SAMPLES = 95

X = [0.000129900000, 0.000232100000, 0.000414900000, 0.000741600000, 0.001368000000,
    0.002236000000, 0.004243000000, 0.007650000000, 0.014310000000, 0.023190000000,
    0.043510000000, 0.077630000000, 0.134380000000, 0.214770000000, 0.283900000000,
    0.328500000000, 0.348280000000, 0.348060000000, 0.336200000000, 0.318700000000,
    0.290800000000, 0.251100000000, 0.195360000000, 0.142100000000, 0.095640000000,
    0.057950010000, 0.032010000000, 0.014700000000, 0.004900000000, 0.002400000000,
    0.009300000000, 0.029100000000, 0.063270000000, 0.109600000000, 0.165500000000,
    0.225749900000, 0.290400000000, 0.359700000000, 0.433449900000, 0.512050100000,
    0.594500000000, 0.678400000000, 0.762100000000, 0.842500000000, 0.916300000000,
    0.978600000000, 1.026300000000, 1.056700000000, 1.062200000000, 1.045600000000,
    1.002600000000, 0.938400000000, 0.854449900000, 0.751400000000, 0.642400000000,
    0.541900000000, 0.447900000000, 0.360800000000, 0.283500000000, 0.218700000000,
    0.164900000000, 0.121200000000, 0.087400000000, 0.063600000000, 0.046770000000,
    0.032900000000, 0.022700000000, 0.015840000000, 0.011359160000, 0.008110916000,
    0.005790346000, 0.004109457000, 0.002899327000, 0.002049190000, 0.001439971000,
    0.000999949300, 0.000690078600, 0.000476021300, 0.000332301100, 0.000234826100,
    0.000166150500, 0.000117413000, 0.000083075270, 0.000058706520, 0.000041509940,
    0.000029353260, 0.000020673830, 0.000014559770, 0.000010253980, 0.000007221456,
    0.000005085868, 0.000003581652, 0.000002522525, 0.000001776509, 0.000001251141]

Y = [0.000003917000, 0.000006965000, 0.000012390000, 0.000022020000, 0.000039000000,
    0.000064000000, 0.000120000000, 0.000217000000, 0.000396000000, 0.000640000000,
    0.001210000000, 0.002180000000, 0.004000000000, 0.007300000000, 0.011600000000,
    0.016840000000, 0.023000000000, 0.029800000000, 0.038000000000, 0.048000000000,
    0.060000000000, 0.073900000000, 0.090980000000, 0.112600000000, 0.139020000000,
    0.169300000000, 0.208020000000, 0.258600000000, 0.323000000000, 0.407300000000,
    0.503000000000, 0.608200000000, 0.710000000000, 0.793200000000, 0.862000000000,
    0.914850100000, 0.954000000000, 0.980300000000, 0.994950100000, 1.000000000000,
    0.995000000000, 0.978600000000, 0.952000000000, 0.915400000000, 0.870000000000,
    0.816300000000, 0.757000000000, 0.694900000000, 0.631000000000, 0.566800000000,
    0.503000000000, 0.441200000000, 0.381000000000, 0.321000000000, 0.265000000000,
    0.217000000000, 0.175000000000, 0.138200000000, 0.107000000000, 0.081600000000,
    0.061000000000, 0.044580000000, 0.032000000000, 0.023200000000, 0.017000000000,
    0.011920000000, 0.008210000000, 0.005723000000, 0.004102000000, 0.002929000000,
    0.002091000000, 0.001484000000, 0.001047000000, 0.000740000000, 0.000520000000,
    0.000361100000, 0.000249200000, 0.000171900000, 0.000120000000, 0.000084800000,
    0.000060000000, 0.000042400000, 0.000030000000, 0.000021200000, 0.000014990000,
    0.000010600000, 0.000007465700, 0.000005257800, 0.000003702900, 0.000002607800,
    0.000001836600, 0.000001293400, 0.000000910930, 0.000000641530, 0.000000451810]

Z = [0.000606100000, 0.001086000000, 0.001946000000, 0.003486000000, 0.006450001000,
    0.010549990000, 0.020050010000, 0.036210000000, 0.067850010000, 0.110200000000,
    0.207400000000, 0.371300000000, 0.645600000000, 1.039050100000, 1.385600000000,
    1.622960000000, 1.747060000000, 1.782600000000, 1.772110000000, 1.744100000000,
    1.669200000000, 1.528100000000, 1.287640000000, 1.041900000000, 0.812950100000,
    0.616200000000, 0.465180000000, 0.353300000000, 0.272000000000, 0.212300000000,
    0.158200000000, 0.111700000000, 0.078249990000, 0.057250010000, 0.042160000000,
    0.029840000000, 0.020300000000, 0.013400000000, 0.008749999000, 0.005749999000,
    0.003900000000, 0.002749999000, 0.002100000000, 0.001800000000, 0.001650001000,
    0.001400000000, 0.001100000000, 0.001000000000, 0.000800000000, 0.000600000000,
    0.000340000000, 0.000240000000, 0.000190000000, 0.000100000000, 0.000049999990,
    0.000030000000, 0.000020000000, 0.000010000000, 0.000000000000, 0.000000000000,
    0.000000000000, 0.000000000000, 0.000000000000, 0.000000000000, 0.000000000000,
    0.000000000000, 0.000000000000, 0.000000000000, 0.000000000000, 0.000000000000,
    0.000000000000, 0.000000000000, 0.000000000000, 0.000000000000, 0.000000000000,
    0.000000000000, 0.000000000000, 0.000000000000, 0.000000000000, 0.000000000000,
    0.000000000000, 0.000000000000, 0.000000000000, 0.000000000000, 0.000000000000,
    0.000000000000, 0.000000000000, 0.000000000000, 0.000000000000, 0.000000000000,
    0.000000000000, 0.000000000000, 0.000000000000, 0.000000000000, 0.000000000000]

def N(x):
    return (x / 10566.864005283874576)

D65 = [N(46.6383), N(49.3637), N(52.0891), N(51.0323), N(49.9755), N(52.3118), N(54.6482),
    N(68.7015), N(82.7549), N(87.1204), N(91.486),  N(92.4589), N(93.4318), N(90.057),
    N(86.6823), N(95.7736), N(104.865), N(110.936), N(117.008), N(117.41),  N(117.812),
    N(116.336), N(114.861), N(115.392), N(115.923), N(112.367), N(108.811), N(109.082),
    N(109.354), N(108.578), N(107.802), N(106.296), N(104.79),  N(106.239), N(107.689),
    N(106.047), N(104.405), N(104.225), N(104.046), N(102.023), N(100.0),   N(98.1671),
    N(96.3342), N(96.0611), N(95.788),  N(92.2368), N(88.6856), N(89.3459), N(90.0062),
    N(89.8026), N(89.5991), N(88.6489), N(87.6987), N(85.4936), N(83.2886), N(83.4939),
    N(83.6992), N(81.863),  N(80.0268), N(80.1207), N(80.2146), N(81.2462), N(82.2778),
    N(80.281),  N(78.2842), N(74.0027), N(69.7213), N(70.6652), N(71.6091), N(72.979),
    N(74.349),  N(67.9765), N(61.604),  N(65.7448), N(69.8856), N(72.4863), N(75.087),
    N(69.3398), N(63.5927), N(55.0054), N(46.4182), N(56.6118), N(66.8054), N(65.0941),
    N(63.3828), N(63.8434), N(64.304),  N(61.8779), N(59.4519), N(55.7054), N(51.959),
    N(54.6998), N(57.4406), N(58.8765), N(60.3125)]

def N(x):
    return (x / 106.8)

E = [N(1.0), N(1.0), N(1.0), N(1.0), N(1.0), N(1.0), N(1.0), N(1.0), N(1.0), N(1.0),
    N(1.0), N(1.0), N(1.0), N(1.0), N(1.0), N(1.0), N(1.0), N(1.0), N(1.0), N(1.0),
    N(1.0), N(1.0), N(1.0), N(1.0), N(1.0), N(1.0), N(1.0), N(1.0), N(1.0), N(1.0),
    N(1.0), N(1.0), N(1.0), N(1.0), N(1.0), N(1.0), N(1.0), N(1.0), N(1.0), N(1.0),
    N(1.0), N(1.0), N(1.0), N(1.0), N(1.0), N(1.0), N(1.0), N(1.0), N(1.0), N(1.0),
    N(1.0), N(1.0), N(1.0), N(1.0), N(1.0), N(1.0), N(1.0), N(1.0), N(1.0), N(1.0),
    N(1.0), N(1.0), N(1.0), N(1.0), N(1.0), N(1.0), N(1.0), N(1.0), N(1.0), N(1.0),
    N(1.0), N(1.0), N(1.0), N(1.0), N(1.0), N(1.0), N(1.0), N(1.0), N(1.0), N(1.0),
    N(1.0), N(1.0), N(1.0), N(1.0), N(1.0), N(1.0), N(1.0), N(1.0), N(1.0), N(1.0),
    N(1.0), N(1.0), N(1.0), N(1.0), N(1.0)]

def N(x):
    return (x / 10503.2)

D50 = [N(23.942000),  N(25.451000),  N(26.961000),  N(25.724000),  N(24.488000),
    N(27.179000),  N(29.871000),  N(39.589000),  N(49.308000),  N(52.910000),
    N(56.513000),  N(58.273000),  N(60.034000),  N(58.926000),  N(57.818000),
    N(66.321000),  N(74.825000),  N(81.036000),  N(87.247000),  N(88.930000),
    N(90.612000),  N(90.990000),  N(91.368000),  N(93.238000),  N(95.109000),
    N(93.536000),  N(91.963000),  N(93.843000),  N(95.724000),  N(96.169000),
    N(96.613000),  N(96.871000),  N(97.129000),  N(99.614000),  N(102.099000),
    N(101.427000), N(100.755000), N(101.536000), N(102.317000), N(101.159000),
    N(100.000000), N(98.868000),  N(97.735000),  N(98.327000),  N(98.918000),
    N(96.208000),  N(93.499000),  N(95.593000),  N(97.688000),  N(98.478000),
    N(99.269000),  N(99.155000),  N(99.042000),  N(97.382000),  N(95.722000),
    N(97.290000),  N(98.857000),  N(97.262000),  N(95.667000),  N(96.929000),
    N(98.190000),  N(100.597000), N(103.003000), N(101.068000), N(99.133000),
    N(93.257000),  N(87.381000),  N(89.492000),  N(91.604000),  N(92.246000),
    N(92.889000),  N(84.872000),  N(76.854000),  N(81.683000),  N(86.511000),
    N(89.546000),  N(92.580000),  N(85.405000),  N(78.230000),  N(67.961000),
    N(57.692000),  N(70.307000),  N(82.923000),  N(80.599000),  N(78.274000),
    N(0),          N(0),          N(0),          N(0),          N(0),
    N(0),          N(0),          N(0),          N(0)]

def N(x):
    return (x / 10536.3)

D60 = [N(38.683115),  N(41.014457),  N(42.717548),  N(42.264182),  N(41.454941),
    N(41.763698),  N(46.605319),  N(59.226938),  N(72.278594),  N(78.231500),
    N(80.440600),  N(82.739580),  N(82.915027),  N(79.009168),  N(77.676264),
    N(85.163609),  N(95.681274),  N(103.267764), N(107.954821), N(109.777964),
    N(109.559187), N(108.418402), N(107.758141), N(109.071548), N(109.671404),
    N(106.734741), N(103.707873), N(103.981942), N(105.232199), N(105.235867),
    N(104.427667), N(103.052881), N(102.522934), N(104.371416), N(106.052671),
    N(104.948900), N(103.315154), N(103.416286), N(103.538599), N(102.099304),
    N(100.000000), N(97.992725),  N(96.751421),  N(97.102402),  N(96.712823),
    N(93.174457),  N(89.921479),  N(90.351933),  N(91.999793),  N(92.384009),
    N(92.098710),  N(91.722859),  N(90.646003),  N(88.327552),  N(86.526483),
    N(87.034239),  N(87.579186),  N(85.884584),  N(83.976140),  N(83.743140),
    N(84.724074),  N(86.450818),  N(87.493491),  N(86.546330),  N(83.483070),
    N(78.268785),  N(74.172451),  N(74.275184),  N(76.620385),  N(79.423856),
    N(79.051849),  N(71.763360),  N(65.471371),  N(67.984085),  N(74.106079),
    N(78.556612),  N(79.527120),  N(75.584935),  N(67.307163),  N(55.275106),
    N(49.273538),  N(59.008629),  N(70.892412),  N(70.950115),  N(67.163996),
    N(67.445480),  N(68.171371),  N(66.466636),  N(62.989809),  N(58.067786),
    N(54.990892),  N(56.915942),  N(60.825601),  N(62.987850)]

xyz_to_srgb = np.array(
    [[3.240479, -1.537150, -0.498535],
    [-0.969256, 1.875991, 0.041556],
    [0.055648, -0.204043, 1.057311]]
)

srgb_to_xyz = np.array(
    [[0.412453, 0.357580, 0.180423],
    [0.212671, 0.715160, 0.072169],
    [0.019334, 0.119193, 0.950227]]
)
