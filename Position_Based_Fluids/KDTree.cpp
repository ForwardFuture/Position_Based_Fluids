#include "KDTree.h"

KDTree t[NUM];

int KDTree::tot = 0;
int KDTree::root = 0;
int KDTree::idseq[NUM];
std::queue<int>KDTree::neighboring;

KDTree::KDTree() {
	id = lc = rc = 0;
	pos = maxn = minn = glm::vec3(0.0f);
}

void KDTree::clear() {
	id = lc = rc = 0;
	pos = maxn = minn = glm::vec3(0.0f);
}

void KDTree::pushup() {
	if (lc) {
		maxn.x = std::max(maxn.x, t[lc].maxn.x);
		maxn.y = std::max(maxn.y, t[lc].maxn.y);
		maxn.z = std::max(maxn.z, t[lc].maxn.z);
		minn.x = std::min(minn.x, t[lc].minn.x);
		minn.y = std::min(minn.y, t[lc].minn.y);
		minn.z = std::min(minn.z, t[lc].minn.z);
	}
	if (rc) {
		maxn.x = std::max(maxn.x, t[rc].maxn.x);
		maxn.y = std::max(maxn.y, t[rc].maxn.y);
		maxn.z = std::max(maxn.z, t[rc].maxn.z);
		minn.x = std::min(minn.x, t[rc].minn.x);
		minn.y = std::min(minn.y, t[rc].minn.y);
		minn.z = std::min(minn.z, t[rc].minn.z);
	}
}

float KDTree::dis(glm::vec3 checkpos) {
	float disx = checkpos.x - pos.x;
	float disy = checkpos.y - pos.y;
	float disz = checkpos.z - pos.z;

	return sqrt(disx * disx + disy * disy + disz * disz);
}

float KDTree::Evaluation(glm::vec3 checkpos) {
	//NOTICE: Before call this function you must ensure the node exists
	float disx = std::min(fabs(checkpos.x - minn.x), fabs(checkpos.x - maxn.x));
	float disy = std::min(fabs(checkpos.y - minn.y), fabs(checkpos.y - maxn.y));
	float disz = std::min(fabs(checkpos.z - minn.z), fabs(checkpos.z - maxn.z));

	return std::min(disx, std::min(disy, disz));
}

bool KDTree::Inside(glm::vec3 checkpos) {
	bool bx = minn.x <= checkpos.x && checkpos.x <= maxn.x;
	bool by = minn.y <= checkpos.y && checkpos.y <= maxn.y;
	bool bz = minn.z <= checkpos.z && checkpos.z <= maxn.z;

	return bx && by && bz;
}

void KDTree::BuildKDTree() {
	KDTree::tot = KDTree::root = 0;
	for (int i = 0; i < NUM; i++)idseq[i] = i;
	KDTree::BuildKDTree(KDTree::root, 0, NUM - 1, 0);
}

void KDTree::BuildKDTree(int &o, int l, int r, int d) {
	if (l > r)return;
	if (!o)o = ++KDTree::tot;
	int mid = (l + r) >> 1;
	std::nth_element(idseq + l, idseq + mid, idseq + r + 1, [=](int& a, int& b) {return particles[a].Pos[d] < particles[b].Pos[d]; });
	t[o].clear();
	t[o].id = idseq[mid];
	t[o].pos = t[o].minn = t[o].maxn = particles[idseq[mid]].Pos;
	if (l == r)return;
	BuildKDTree(t[o].lc, l, mid - 1, (d + 1) % 3);
	BuildKDTree(t[o].rc, mid + 1, r, (d + 1) % 3);
	t[o].pushup();
}

void KDTree::FindNeighbors(glm::vec3 checkpos, float h) {
	while (!KDTree::neighboring.empty())KDTree::neighboring.pop();
	KDTree::FindNeighbors(KDTree::root, checkpos, h);
}

void KDTree::FindNeighbors(int o, glm::vec3 checkpos, float h) {
	//QUESTION: Why the time complexity is right?
	if (!o)return;

	if (t[o].dis(checkpos) < h)
		neighboring.push(t[o].id);

	float lv, rv;
	if (t[o].lc)lv = t[t[o].lc].Evaluation(checkpos);
	else lv = FLOAT_INF;
	if (t[o].rc)rv = t[t[o].rc].Evaluation(checkpos);
	else rv = FLOAT_INF;
	
	if (lv < h || (t[o].lc && t[t[o].lc].Inside(checkpos)))FindNeighbors(t[o].lc, checkpos, h);
	if (rv < h || (t[o].rc && t[t[o].rc].Inside(checkpos)))FindNeighbors(t[o].rc, checkpos, h);
}