#include "common.hpp"

/**
* netvar_tree - Constructor
*
* Call populate_nodes on every RecvTable under client->GetAllClasses()
*/
void netvar_tree::init()
{
	const auto *client_class = g_IBaseClient->GetAllClasses();
	while(client_class != nullptr)
	{
		const auto class_info = std::make_shared<node>(0, nullptr);
		auto *recv_table = client_class->m_pRecvTable;
		populate_nodes(recv_table, &class_info->nodes);
		nodes.emplace(recv_table->GetName(), class_info);
		client_class = client_class->m_pNext;
	}
}

/**
* populate_nodes - Populate a node map with brances
* @recv_table:	Table the map corresponds to
* @map:	Map pointer
*
* Add info for every prop in the recv table to the node map. If a prop is a
* datatable itself, initiate a recursive call to create more branches.
*/
void netvar_tree::populate_nodes(RecvTable *recv_table, map_type *map)
{
	map->clear();
	for(auto i = 0; i < recv_table->GetNumProps(); i++)
	{
		const auto *prop = recv_table->GetProp(i);
		const auto prop_info = std::make_shared<node>(prop->GetOffset(), const_cast<RecvProp *>(prop));
		if(prop->GetType() == DPT_DataTable) {
			populate_nodes(prop->GetDataTable(), &prop_info->nodes);
		}
		//(*map)[prop->GetName()] = std::shared_ptr<node>(prop_info);
		map->emplace(prop->GetName(), prop_info);
	}
}

netvar_tree gNetvars;
