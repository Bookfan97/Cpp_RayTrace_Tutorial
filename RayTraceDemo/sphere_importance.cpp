//#include "sphere_importance.h"
//
//
//#include <iomanip>
//#include <iostream>
//
//
//#include "pi.h"
//#include "vec3.h"
//
//double sphere_importance::pdf(double x)
//{
//	    return 1 / (4*pi);
//}
//
//void sphere_importance::calcSphereImportance()
//{
//	    int N = 1000000;
//    auto sum = 0.0;
//    for (int i = 0; i < N; i++) {
//        vec3 d = random_unit_vector();
//        auto cosine_squared = d.z()*d.z();
//  //      sum += cosine_squared / pdf(d);
//    }
//    std::cout << std::fixed << std::setprecision(12);
//    std::cout << "I = " << sum/N << '\n';
//}
