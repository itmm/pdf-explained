#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct Object {
	struct Object *link;
	int id;
	int offset;
};

static int byteOffset = 0;
static int maxObjectId = 0;
static struct Object *allObjects = NULL;
	
static void write(const char *str) {
	printf("%s\n", str);
	byteOffset += strlen(str);
}

static inline void writeHeader() {
	write("%PDF-1.0");
	write("%\xe2\xe3\xcf\xd3");
}

static void writeObjectHeader(int id) {
	struct Object *obj = malloc(sizeof(struct Object));
	assert(obj); if (! obj) { exit(10); }
	obj->link = allObjects;
	obj->id = id;
	obj->offset = byteOffset;
	allObjects = obj;
	if (id > maxObjectId) { maxObjectId = id; }

	char buffer[20];
	snprintf(buffer, sizeof(buffer), "%d 0 obj", id);
	write(buffer);
}

static void writeObject(int id, const char *data) {
	writeObjectHeader(id);
	write(data);
	write("endobj");
}

static void writeStream(int id, const char *stream) {
	writeObjectHeader(id);
	write("<<");
	char buffer[20];
	snprintf(buffer, sizeof(buffer), "/Length %d", (int) strlen(stream));
	write(buffer);
	write(">>");
	write("stream");
	write(stream);
	write("endstream");
	write("endobj");
}

static int xrefOffset = 0;

static struct Object *findObject(int id) {
	for (struct Object *obj = allObjects; obj; obj = obj->link) {
		if (obj->id == id) { return obj; }
	}
	return NULL;
}

static void writeXref() {
	xrefOffset = byteOffset;
	write("xref");
	char buffer[20];
	snprintf(buffer, sizeof(buffer), "0 %d", maxObjectId + 1);
	write(buffer);
	for (int i = 0; i <= maxObjectId; ++i) {
		struct Object *obj = findObject(i);
		if (obj) { 
			snprintf(buffer, sizeof(buffer), "%010d %05d n", obj->offset, 0);
			write(buffer);
		} else {
			write("0000000000 65535 f");
		}
	}
}

static void writeTrailer(int rootId) {
	write("trailer");
	char buffer[40];
	snprintf(
		buffer,
		sizeof(buffer),
		"<<"
			"/Root %d 0 R "
			"/Size %d"
		">>",
		rootId,
		maxObjectId + 1
	);
	write(buffer);
	write("startxref");
	snprintf(buffer, sizeof(buffer), "%d", xrefOffset);
	write(buffer);
	write("%%EOF");
}

int main()
{
	writeHeader();
	writeObject(
		1,
		"<<"
			"/Kids [2 0 R] "
			"/Type /Pages "
			"/Count 1"
		">>"
	);
	writeObject(
		2,
		"<<"
			"/Resources 3 0 R "
			"/Contents [4 0 R] "
			"/Parent 1 0 R "
			"/Type /Page "
			"/MediaBox [0 0 792 612]"
		">>"
	);
	writeObject(
		3,
		"<<"
			"/Pattern <<"
				"/GradientShading <<"
					"/Type /Pattern "
					"/PatternType 2 "
					"/Shading <<"
						"/ColorSpace /DeviceGray "
						"/ShadingType 3 "
						"/Function <<"
							"/FunctionType 2 "
							"/N 1 "
							"/Domain [0 1]"
						">> "
						"/Coords [400 400 0 400 400 200] "
						"/Extend [true true]"
					">> "
				">> "
			">> "
		">>"
	);
	writeStream(
		4,
		"/Pattern cs "
		"/GradientShading scn "
		"250 300 m 350 500 450 500 550 300 c "
		"450 300 350 250 y h f"
	);
	writeObject(
		5,
		"<<"
			"/Type /Catalog "
			"/Pages 1 0 R"
		">>"
	);
	writeXref();
	writeTrailer(5);
}
