class Edge
{
	int32_t id_u;
	int32_t id_v;
	uint8_t rating;
	std::vector<int32_t> profile_u;
	std::vector<int32_t> profile_v;
	bool isReal;
	uint32_t aMac_u;
	uint32_t aMac_v;
	uint32_t aMac_data;
	uint32_t bMac_data;
};

