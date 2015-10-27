#include <stdbool.h>
#include "client_DS.h"
#include "print.h"

/* <doc>
 * void display_moderator_pending_list(client_information_t **client_info, moderator_show_type_t show_type)
 * This is a MODERATOR function, for displaying the contents of pending and done client lists.
 * It takes argument as show_type which can be -
 * SHOW_MOD_PENDING_CLIENTS
 * SHOW_MOD_DONE_CLIENTS
 *
 * </doc>
 */
void display_moderator_pending_list(client_information_t **client_info, moderator_show_type_t show_type)
{
  mod_client_node_t *mod_node = NULL;
  char buf[100];
  char clientIP[INET_ADDRSTRLEN];

  /*If client is not a moderator*/
  if ((*client_info)->moderator_info == NULL) {
    SIMPLE_PRINT("\t Client is not a moderator.\n");
    return;
  }

  if (show_type == SHOW_MOD_PENDING_CLIENTS) {
      mod_node =     SN_LIST_MEMBER_HEAD(&((*client_info)->moderator_info->pending_client_list->client_grp_node),
                                         mod_client_node_t,
                                         list_element);
  } else {
      mod_node =     SN_LIST_MEMBER_HEAD(&((*client_info)->moderator_info->done_client_list->client_grp_node),
                                         mod_client_node_t,
                                         list_element);
  }

  /*If no nodes associated with the list*/
  if (mod_node == NULL)
  {
    SIMPLE_PRINT("\n\t  Moderator has no clients in the list.\n");
    return;
  }

  
  PRINT("Client Address \t\t\t\t Heartbeats Missed");
  PRINT("----------------------------------------------------------------------------------");

  while (mod_node)
  {
     inet_ntop(AF_INET, get_in_addr(&(mod_node->peer_client_addr)), clientIP, INET6_ADDRSTRLEN);

     sprintf(buf,
             "\n\t%s \t\t\t\t\t %d",
             clientIP,
             mod_node->heartbeats_missed);
     SIMPLE_PRINT(buf);

     mod_node  =    SN_LIST_MEMBER_NEXT(mod_node,
                                        mod_client_node_t,
                                        list_element);
  }

}

/* <doc>
 * void deallocate_moderator_list(moderator_information_t **moderator_info)
 * Deallocates the moderator list.
 *
 * </doc>
 */
void deallocate_moderator_list(client_information_t **client_info)
{
   mod_client_node_t *mod_node = NULL;
   moderator_information_t *mod_info = (*client_info)->moderator_info;

   /*Deallocate all nodes from done list first*/
   mod_node =     SN_LIST_MEMBER_HEAD(&(mod_info->done_client_list->client_grp_node),
                                         mod_client_node_t,
                                         list_element);

   while (mod_node)
   {
      SN_LIST_MEMBER_REMOVE(&(mod_info->done_client_list->client_grp_node),
                            mod_node,
                            list_element);
      free(mod_node);

      mod_node =     SN_LIST_MEMBER_HEAD(&(mod_info->done_client_list->client_grp_node),
                                         mod_client_node_t,
                                         list_element);
   }

   /*deallocate the pending and done lists*/
   free(mod_info->pending_client_list);
   free(mod_info->done_client_list);
//   (*moderator_info)->pending_client_list = NULL;
//   (*moderator_info)->done_client_list = NULL;
   /*free the moderator_info*/
   free(mod_info);
   (*client_info)->moderator_info = NULL;
}

/* <doc>
 * void move_moderator_node_pending_to_done_list(moderator_information_t **moderator_info, mod_client_node_t *mod_node)
 * This is a helper function, which move the moderator client node from pending list to done list.
 *
 * </doc>
 */
void move_moderator_node_pending_to_done_list(moderator_information_t **moderator_info, mod_client_node_t *mod_node)
{
   /*Insert in done list*/
   SN_LIST_MEMBER_INSERT_HEAD(&((*moderator_info)->done_client_list->client_grp_node),
                              mod_node,
                              list_element);

   /*Remove from pending list*/
   SN_LIST_MEMBER_REMOVE(&((*moderator_info)->pending_client_list->client_grp_node),
                         mod_node,
                         list_element);  
}

/* <doc>
 * mod_client_node_t *allocate_clnt_moderator_node(moderator_information_t **moderator_info)
 * Allocates the moderator client node and inserts into pending client list.
 *
 * </doc>
 */
mod_client_node_t *allocate_clnt_moderator_node(moderator_information_t **moderator_info)
{
   mod_client_node_t *mod_node = NULL;

   mod_node = malloc(sizeof(mod_client_node_t));

   mod_node->peer_client_id = 0;
   mod_node->heartbeats_missed = 0;

   SN_LIST_MEMBER_INSERT_HEAD(&((*moderator_info)->pending_client_list->client_grp_node),
                              mod_node,
                              list_element);

   return mod_node;
}

void allocate_moderator_list(moderator_information_t **moderator_info)
{
   client_grp_t  *list1 = NULL, *list2 = NULL;
   list1  = malloc(sizeof(client_grp_t));
   SN_LIST_INIT(&(list1->client_grp_node));
   list2  = malloc(sizeof(client_grp_t));
   SN_LIST_INIT(&(list2->client_grp_node));
   (*moderator_info)->pending_client_list = list1;
   (*moderator_info)->done_client_list = list2;
}

/* <doc>
 * void allocate_moderator_info(client_information_t **client_info)
 * Allocates the moderator info list and stores the pointer in
 * client info
 *
 * </doc
 */
void allocate_moderator_info(client_information_t **client_info)
{
   moderator_information_t *mod_info = (moderator_information_t *) malloc(sizeof(moderator_information_t));
   mod_info->fsm_state = STATE_NONE;
   (*client_info)->moderator_info = mod_info;
   allocate_moderator_list(&(*client_info)->moderator_info);
}

void allocate_client_grp_list(client_information_t **client_info)
{
   client_grp_t  *mcast_client = NULL;
   mcast_client  = malloc(sizeof(client_grp_t));
   SN_LIST_INIT(&(mcast_client->client_grp_node));
   (*client_info)->client_grp_list = mcast_client;
}

/* <doc>
 * void allocate_client_info(client_information_t **client_info)
 * Allocates and initializes the main linked list - client_info
 * </doc>
 */
void allocate_client_info(client_information_t **client_info)
{
   *client_info = (client_information_t *) malloc(sizeof(client_information_t));
   (*client_info)->client_fd = INT_MAX;
   (*client_info)->epoll_fd = INT_MAX;
   (*client_info)->epoll_evt = NULL;
   allocate_client_grp_list(client_info);
}

/* <doc>
 * client_grp_node_t *allocate_client_grp_node(client_information_t **client_info)
 * This function allocates the node, inserts it into client_info list and 
 * returns node to the caller.
 * 
 * </doc>
 */
client_grp_node_t *allocate_client_grp_node(client_information_t **client_info)
{
   client_grp_node_t *new_client_grp_node = NULL;

   if ((*client_info)->client_grp_list == NULL)
   {
      allocate_client_grp_list(client_info);
   }

   new_client_grp_node = malloc(sizeof(client_grp_node_t));

   new_client_grp_node->group_port = -1;
   new_client_grp_node->mcast_fd = INT_MAX;
   memset(&(new_client_grp_node->group_name), '\0', sizeof(new_client_grp_node->group_name));

   SN_LIST_MEMBER_INSERT_HEAD(&((*client_info)->client_grp_list->client_grp_node),
                             new_client_grp_node,
                             list_element);

   return new_client_grp_node;
}

/* <doc>
 * void deallocate_client_grp_node(client_information_t *client_info, client_grp_node_t *node)
 * This function removes the specied node from the client_info list.
 * 
 * </doc>
 */

void deallocate_client_grp_node(client_information_t *client_info, client_grp_node_t *node)
{
   SN_LIST_MEMBER_REMOVE(&(client_info->client_grp_list->client_grp_node),
                        node,
                        list_element);
   free(node);
}

/* <doc>
 * void display_client_grp_node(client_information_t **client_info)
 * This function traverses and prints the client_info linked list.
 *
 * </doc>
 */
void display_client_grp_node(client_information_t **client_info)
{
  client_grp_node_t *client_grp_node = NULL;
  char buf[100];
  char groupIP[INET_ADDRSTRLEN];


  client_grp_node =     SN_LIST_MEMBER_HEAD(&((*client_info)->client_grp_list->client_grp_node),
                                        client_grp_node_t,
                                        list_element);

  if (client_grp_node == NULL)
  {
    SIMPLE_PRINT("\t  Client has 0 groups.\n");
    return;
  }

  sprintf(buf,"Grp Name  \t Grp IP \t Grp Port \t Mcast_fd \t client_fd");
  PRINT(buf);
  sprintf(buf,"---------------------------------------------------------------------------");
  PRINT(buf);

  while (client_grp_node)
  {
     inet_ntop(AF_INET, get_in_addr((struct sockaddr *)&(client_grp_node->group_addr)), groupIP, INET6_ADDRSTRLEN);

     sprintf(buf,
             "\n\t%s \t\t %s \t\t %d \t\t %d \t\t %d",
             client_grp_node->group_name, groupIP,
             client_grp_node->group_port, client_grp_node->mcast_fd,
             (*client_info)->client_fd);
     SIMPLE_PRINT(buf);

     client_grp_node =     SN_LIST_MEMBER_NEXT(client_grp_node,
                                          client_grp_node_t,
                                          list_element);
  }

}

/* <doc>
 * client_grp_node_t* get_client_grp_node_by_group_name(client_information_t **client_info, char *grp_name)
 * This is the internal function, called by IS_GROUP_IN_CLIENT_LL. This function searches for the
 * node and return it.
 *
 * </doc>
 */
void get_client_grp_node_by_group_name(client_information_t **client_info, char *grp_name, client_grp_node_t **clnt_node)
{

  client_grp_node_t *client_grp_node = NULL;

  client_grp_node =     SN_LIST_MEMBER_HEAD(&((*client_info)->client_grp_list->client_grp_node),
                                            client_grp_node_t,
                                            list_element);
  while (client_grp_node)
  {

     if (strncmp(client_grp_node->group_name,grp_name,strlen(grp_name)) == 0)
     {
        *clnt_node = client_grp_node;
        return;
     }

     client_grp_node =     SN_LIST_MEMBER_NEXT(client_grp_node,
                                               client_grp_node_t,
                                               list_element);
  }

}


/* <doc>
 * bool ADD_CLIENT_IN_LL(client_information_t **client_info, client_grp_node_t *node)
 * Add the client node in client_info list. client node contains information related
 * to multicast group IP/port and other related information.
 * Returns TRUE if added successfully, otherwise FALSE.
 *
 * </doc>
 */
bool ADD_CLIENT_IN_LL(client_information_t **client_info, client_grp_node_t *node)
{
  client_grp_node_t *client_grp_node = NULL;

  client_grp_node = allocate_client_grp_node(client_info);

  if (client_grp_node)
  {
    strcpy(client_grp_node->group_name,node->group_name);
    memcpy(&client_grp_node->group_addr, &node->group_addr, sizeof(client_grp_node->group_addr));
    client_grp_node->group_port = node->group_port;
    client_grp_node->mcast_fd = node->mcast_fd;

    return TRUE;
  }

  return FALSE;
}

/* <doc>
 * bool IS_GROUP_IN_CLIENT_LL(client_information_t **client_info, char *group_name)
 * Traverses the client_info list and checks for node with group_name.
 * Returns TRUE if node is found, Otherwise FALSE.
 *
 * </doc>
 */
bool IS_GROUP_IN_CLIENT_LL(client_information_t **client_info, char *group_name)
{
  client_grp_node_t *client_grp_node = NULL;

  get_client_grp_node_by_group_name(client_info, group_name, &client_grp_node);

  if (client_grp_node)
    return TRUE;

  return FALSE;
}