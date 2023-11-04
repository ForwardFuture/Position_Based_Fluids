#pragma once

#include "Global.h"
#include "particles.h"

class KDTree {
private:
	//STATIC
	static int tot;
	static int root;
	static int idseq[NUM];

	static void BuildKDTree(int& o, int l, int r, int d);
	static void FindNeighbors(int o, glm::vec3 checkpos, float h);

	//NON_STATIC
	int id;
	int lc, rc;
	glm::vec3 pos, maxn, minn;

	void clear();
	void pushup();
	float dis(glm::vec3 checkpos);
	float Evaluation(glm::vec3 checkpos);
	bool Inside(glm::vec3 checkpos);
public:
	//STATIC
	static std::queue<int>neighboring;

	static void BuildKDTree();
	static void FindNeighbors(glm::vec3 checkpos, float h);

	//NON_STATIC
	KDTree();

};

extern KDTree t[NUM];