
// written by shinpei nakata

K.import("konoha.float");
K.import("konoha.math");
K.import("konoha");
K.import("konoha.bytes");
K.import("konoha.string");
K.import("posix.file");

int width = 64;
int height = 64;

int nsubsamples = 2;
int naosamples = 4;

class Vec {
	float x;
	float y;
	float z;
	Vec (float x0, float y0, float z0) {
		this.x = x0;
		this.y = y0;
		this.z = z0;
	}
}

float dot (Vec v0, Vec v1) {
	return v0.x * v1.x + v0.y * v1.y + v0.z * v1.z;
}

void cross(Vec v0, Vec v1, Vec v2) {
	v2.x = v0.y * v1.z - v0.z * v1.y;
	v2.y = v0.z * v1.x - v0.x * v1.z;
	v2.z = v0.x * v1.y - v0.y * v1.x;
}

void normalize(Vec v1) {
	double q = dot(v1, v1);
	double length = Math.sqrt(q);
	if (length > 1.0e-10) {
		v1.x = v1.x / length;
		v1.y = v1.y / length;
		v1.z = v1.z / length;
	}
}


class RayIntersection {
	Vec rayPosition;
	Vec rayDirection;
	Vec hitPosition;
	Vec normal;
	double distance;
	boolean  hit;
}

class Sphere {
	Vec center;
	double radius;

	Sphere(double x0, double y0, double z0, double r0) {
		this.center = new Vec(x0, y0, z0);
		this.radius = r0;
	}

	Array[Sphere] newArray(int size) {
		return this;
	}
}

void interSectsSphere(Sphere s, RayIntersection isect) {
	Vec rs = new Vec(isect.rayPosition.x - s.center.x, isect.rayPosition.y - s.center.y, isect.rayPosition.z - s.center.z);
	double b = dot(rs, isect.rayDirection);
	double c = dot(rs, rs) - s.radius * s.radius;
	double d = b * b - c;

	if (d > 0.0) {
		double t = -b - Math.sqrt(d);
		if ((t > 0.0) && (t < isect.distance)) {
			isect.distance = t;
			isect.hit = true;

			isect.hitPosition.x = isect.rayPosition.x + isect.rayDirection.x * t;
			isect.hitPosition.y = isect.rayPosition.y + isect.rayDirection.y * t;
			isect.hitPosition.z = isect.rayPosition.z + isect.rayDirection.z * t;

			isect.normal.x = isect.hitPosition.x - s.center.x;
			isect.normal.y = isect.hitPosition.y - s.center.y;
			isect.normal.z = isect.hitPosition.z - s.center.z;

			normalize(isect.normal);
		}
	}
}

class Plane {
	Vec position;
	Vec normal;
	Plane () {
	}
}

Array[Sphere] spheres;
Plane plane;

void interSectsPlane(Plane plane, RayIntersection isect) {
	double d = -1.0 * dot(plane.position, plane.normal);
	double v = dot(isect.rayDirection, plane.normal);

	if (v > 0.0000000000000001 || v < -0.00000000000000001) {
		double t = -1.0 * (dot(isect.rayPosition, plane.normal) + d) / v;

		if ((t > 0.0) && (t < isect.distance)) {
			isect.distance = t;
			isect.hit = true;
			isect.hitPosition.x = isect.rayPosition.x + isect.rayDirection.x * t;
			isect.hitPosition.y = isect.rayPosition.y + isect.rayDirection.y * t;
			isect.hitPosition.z = isect.rayPosition.z + isect.rayDirection.z * t;
			isect.normal.x = plane.normal.x;
			isect.normal.y = plane.normal.y;
			isect.normal.z = plane.normal.z;
		}
	}
}

void getOrthoBasis (Vec normal, Vec[] orthoBasis) {
	orthoBasis[0] = new Vec(0.0, 0.0, 0.0));
	orthoBasis[1] = new Vec(0.0, 0.0, 0.0));
	orthoBasis[2] = new Vec(0.0, 0.0, 0.0));
	Vec o0 = orthoBasis[0];
	Vec o1 = orthoBasis[1];
	Vec o2 = orthoBasis[2];

	o2.x = normal.x;
	o2.y = normal.y;
	o2.z = normal.z;

	if ((normal.x < 0.6) && (normal.x > -0.6)) {
		o1.x  = 1.0;
	} else if ((normal.y < 0.6) && (normal.y > -0.6)) {
		o1.y = 1.0;
	} else if ((normal.z < 0.6) && (normal.z > -0.6)) {
		o1.z = 1.0;
	}else {
		o1.x = 1.0;
	}
	cross(o1, o2, o0);
	normalize(o0);
	cross(o2, o0, o1);
	normalize(o1);
}

void getAmbientOcclusion(RayIntersection isect, Vec ambientOcclusion) {
	int i, j;
	int ntheta = naosamples;
	int nphi = naosamples;
	double eps = 0.0001;
	double pi = 3.141593;

	RayIntersection occlsect = new RayIntersection();
	occlsect.rayPosition = new Vec(isect.hitPosition.x + eps * isect.normal.x,isect.hitPosition.y + eps * isect.normal.y,isect.hitPosition.z + eps * isect.normal.z);
	occlsect.rayDirection = new Vec(0.0, 0.0, 0.0);
	occlsect.hitPosition = new Vec(0.0, 0.0, 0.0);
	occlsect.normal = new Vec(0.0, 0.0, 0.0);

	Vec[] basis = new Vec[3];

	getOrthoBasis(isect.normal, basis);

	int hitCount = 0;
	while (j < ntheta) {
		i = 0;
		while (i < nphi) {
			double theta = Math.sqrt(Math.random());
			double phi = 2.0 * pi * Math.random();
			double x = Math.cos(phi) * theta;
			double y = Math.sin(phi) * theta;
			double z = Math.sqrt(1.0 - theta * theta);
			occlsect.rayDirection.x = x * basis[0].x + y * basis[1].x + z * basis[2].x;
			occlsect.rayDirection.y = x * basis[0].y + y * basis[1].y + z * basis[2].y;
			occlsect.rayDirection.z = x * basis[0].z + y * basis[1].z + z * basis[2].z;
			occlsect.distance = 100000000000;
			occlsect.hit = false;

			interSectsSphere(spheres[0], occlsect);
			interSectsSphere(spheres[1], occlsect);
			interSectsSphere(spheres[2], occlsect);
			interSectsPlane(plane,occlsect);

			if (occlsect.hit) {
				hitCount += 1;
			}
			i += 1;
		} 
		j += 1;
	}
	double fntheta = ntheta;
	double fnphi = nphi;
	double fhitCount = hitCount;
	double occlusionRatio = (fntheta * fnphi - fhitCount) / (fntheta * fnphi);
	ambientOcclusion.x = occlusionRatio;
	ambientOcclusion.y = occlusionRatio;
	ambientOcclusion.z = occlusionRatio;
}

int clamp(double value) {
	int i = value * 255.5;
	if (i > 255) i = 255;
	else if (i < 0) i = 0;
	return i;
}

void render(int[] byteImage, int width, int height, int numberOfSubSamples) {
	float[] fimg = new Float[width * height * 3];
	RayIntersection isect = new RayIntersection();
	int y, x,v,u;
	float fnumberOfSubSamples = numberOfSubSamples;
	isect.rayPosition = new Vec(0.0, 0.0, 0.0);
	isect.rayDirection = new Vec(0.0, 0.0, 0.0);
	isect.hitPosition = new Vec(0.0, 0.0, 0.0);
	isect.normal = new Vec(0.0, 0.0, 0.0);
	float fwidth = width;
	float fheight = height;

	while (y < height){
		float fy = y;
		x = 0;
		while (x < width) {
			float fx = x;
			v = 0;
			while (v < numberOfSubSamples){
				float fv = v;
				u = 0;
				while (u < numberOfSubSamples) {
					float fu = u;
					isect.rayDirection.x = (fx + (fu / fnumberOfSubSamples) - (fwidth / 2.0)) / (fwidth / 2.0);
					isect.rayDirection.y = -(fy + (fv / fnumberOfSubSamples) - (fheight / 2.0)) / (fheight / 2.0);
					isect.rayDirection.z = -1.0;
					isect.distance = 1000000000;
					isect.hit = false;
					normalize(isect.rayDirection);
					interSectsSphere(spheres[0], isect);
					interSectsSphere(spheres[1], isect);
					interSectsSphere(spheres[2], isect);
					interSectsPlane(plane, isect);

					if (isect.hit == true) {
						Vec ambientOcclusion = new Vec(0.0, 0.0, 0.0);
						getAmbientOcclusion(isect, ambientOcclusion);

						int p = 3 * (fy * fwidth + fx);
						fimg[p] = fimg[p] + ambientOcclusion.x;
						fimg[p+1] = fimg[p+1] + ambientOcclusion.y;
						fimg[p+2] = fimg[p+2] + ambientOcclusion.z;
					}
					u += 1;
				}
				v += 1;
			}
			int p1 = 3 * (fy * fwidth + fx);
			int num = numberOfSubSamples * numberOfSubSamples;
			float fnum = num;

			fimg[p1] = fimg[p1] / fnum;
			fimg[p1 + 1] = fimg[p1 + 1] / fnum;
			fimg[p1 + 2] = fimg[p1 + 2] / fnum;
			byteImage[p1] = clamp(fimg[p1]);
			byteImage[p1 + 1] = clamp(fimg[p1 +1]);
			byteImage[p1 + 2] = clamp(fimg[p1 + 2]);
			x += 1;
		}
		y += 1;
	}
}

void setupScene() {
	spheres.add(new Sphere(-2.0, 0.0, -3.5, 0.5));
	spheres.add(new Sphere(-0.5, 0.0, -3.0, 0.5));
	spheres.add(new Sphere(1.0, 0.0, -2.2, 0.5));

	plane.position = new Vec(0.0, -0.5, 0.0);
	plane.normal = new Vec(0.0, 1.0, 0.0);
}

void run() {
	int[] img = new int[width * height * 3];
	int size = width * height * 3;
	int i = 0;
	setupScene();
	render(img, width, height, nsubsamples);

	p("P3");
	p(width);
	p(height);
	p("255");
	while (i < size) {
		String simg = " " + img[i];
		p(simg);
		i += 1;
	}
	p("");

	//FILE outfile = System.fopen("ao.ppm", "w");
	//outfile.write("P3".toBytes(), 0, 2);
	//outfile.write("\n".toBytes(), 0, 1);
	//String swidth = "" + width;
	//outfile.write(swidth.toBytes(), 0, swidth.getSize());
	//String sheight = " " + height;
	//outfile.write(sheight.toBytes(), 0, sheight.getSize());
	//outfile.write("\n".toBytes(), 0, 1);
	//outfile.write("255".toBytes(), 0, 3);
	//while (i < size) {
	//	String simg = " " + img[i];
	//	outfile.write(simg.toBytes(), 0, simg.getSize());
	//	i += 1;
	//}
	//outfile.write("\n".toBytes(), 0, 1);

	//outfile.close();
}

void main() {
	spheres = new Sphere[0];
	plane = new Plane();
	run();
}

main()
