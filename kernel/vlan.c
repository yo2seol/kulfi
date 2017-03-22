#include "vlan.h"

/* Set bits of VLAN header */
u_int32_t set_vlan_bits(u_int16_t in, u_int16_t out, bool inner_most_tag)
{
    u_int32_t result = 0;
    u_int16_t val = ((u_int8_t) in << 8) | ((u_int8_t) out);
    result |= htons(val & 0x0fff);
    // Shift first and then set the ether type
    if (inner_most_tag) {
        result |= htons(ETH_P_IP) << 16;
    }
    else {
        result |= htons(ETH_P_8021Q) << 16;
    }
    return result;
}

/* Push one vlan tag */
void set_vlan(struct sk_buff * skb, u_int16_t in, u_int16_t out, bool inner_most_tag) {
    struct vlan_label * vlanlabel;
    if (!skb) {
        pr_debug("mod_vlan: ERROR set_vlan skb is null.\n");
        return;
    }
    vlanlabel = (struct vlan_label*)skb_push(skb, sizeof(struct vlan_label));
    if (!vlanlabel) {
        pr_debug("mod_vlan: ERROR skb_vlan skb_push failed.\n");
        return;
    }
    (*(u_int32_t*)(vlanlabel)) = set_vlan_bits(in, out, inner_most_tag);
}

u_int32_t set_ztn_length_bits(u_int16_t header_length)
{
    u_int32_t result = 0;
    result |= htons((header_length & 0x0fff));
    result |= htons(ETH_P_8021Q) << 16;
    return result;
}

/* Push a single zero_touch length */
void set_ztn_length(struct sk_buff * skb, u_int16_t header_length) {
    struct vlan_label * vlanlabel;
    if (!skb) {
        pr_debug("mod_vlan: ERROR set_vlan skb is null.\n");
        return;
    }
    vlanlabel = (struct vlan_label*)skb_push(skb, sizeof(struct vlan_label));
    if (!vlanlabel) {
        pr_debug("mod_vlan: ERROR skb_vlan skb_push failed.\n");
        return;
    }
    (*(u_int32_t*)(vlanlabel)) = set_ztn_length_bits(header_length);
}

u_int32_t set_ztn_header_bits()
{
    u_int32_t result = 0;
    u_int16_t protocol_type = 0x01;
    result |= htons((protocol_type & 0x0fff));
    result |= htons(ETH_P_8021Q) << 16;
    return result;
}

/* Push a single zero_touch header */
void set_ztn_head(struct sk_buff * skb) {
    struct vlan_label * vlanlabel;
    if (!skb) {
        pr_debug("mod_vlan: ERROR set_vlan skb is null.\n");
        return;
    }
    vlanlabel = (struct vlan_label*)skb_push(skb, sizeof(struct vlan_label));
    if (!vlanlabel) {
        pr_debug("mod_vlan: ERROR skb_vlan skb_push failed.\n");
        return;
    }
    (*(u_int32_t*)(vlanlabel)) = set_ztn_header_bits();
}

/* Push a vlan stack (list of vlan tags) */
bool set_vlan_stack_static(struct sk_buff *skb, u_int16_t *tags, int stk_len){
    bool inner_most_tag = true;
    bool pushed = false;
    u_int16_t total_len;
    if (tags == NULL) {
        pr_debug("set_vlan_stack_static: Empty tags! stk_len=%d\n", stk_len);
    }
    else{
        stk_len -= 2;
        total_len = stk_len;
        stk_len *= 2;
        while(stk_len > 0){
            set_vlan(skb, tags[stk_len - 2], tags[stk_len - 1], inner_most_tag);
            inner_most_tag = false;
            pushed = true;
            stk_len -= 2;
        }
        // Append the size
        set_ztn_length(skb, total_len); 
        set_ztn_head(skb); 
    }
    return pushed;
}

/* Wrapper over set_vlan_stack_static to handle empty stacks */
bool set_vlan_stack(struct sk_buff * skb, struct stack* stk){
    bool pushed = false;
    pr_debug("enter set_vlan_stack");
    if (stk == NULL || stk->num_tags < 1){
        pr_debug("set_vlan_stack: Empty stack!\n");
    }
    else{
        pushed = set_vlan_stack_static(skb, stk->tags, stk->num_tags);
    }
    pr_debug("exit set_vlan_stack");
    return pushed;
}

/*
int pop_vlan(struct sk_buff *skb){
    return skb_vlan_pop(skb);
}

int push_vlan(struct sk_buff *skb, u_int16_t vlan_id){
    return skb_vlan_push(skb, ntohs(ETH_P_8021Q), vlan_id);
}
*/
