case 0x00: return ocarina::vector<float>{_node->average()[0],_node->average()[0]};
case 0x01: return ocarina::vector<float>{_node->average()[0],_node->average()[1]};
case 0x02: return ocarina::vector<float>{_node->average()[0],_node->average()[2]};
case 0x03: return ocarina::vector<float>{_node->average()[0],_node->average()[3]};
case 0x10: return ocarina::vector<float>{_node->average()[1],_node->average()[0]};
case 0x11: return ocarina::vector<float>{_node->average()[1],_node->average()[1]};
case 0x12: return ocarina::vector<float>{_node->average()[1],_node->average()[2]};
case 0x13: return ocarina::vector<float>{_node->average()[1],_node->average()[3]};
case 0x20: return ocarina::vector<float>{_node->average()[2],_node->average()[0]};
case 0x21: return ocarina::vector<float>{_node->average()[2],_node->average()[1]};
case 0x22: return ocarina::vector<float>{_node->average()[2],_node->average()[2]};
case 0x23: return ocarina::vector<float>{_node->average()[2],_node->average()[3]};
case 0x30: return ocarina::vector<float>{_node->average()[3],_node->average()[0]};
case 0x31: return ocarina::vector<float>{_node->average()[3],_node->average()[1]};
case 0x32: return ocarina::vector<float>{_node->average()[3],_node->average()[2]};
case 0x33: return ocarina::vector<float>{_node->average()[3],_node->average()[3]};