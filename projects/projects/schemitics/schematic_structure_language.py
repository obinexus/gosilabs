# schematic_structure_language.py
from typing import Dict, List, Any
import plantuml
import json
from dataclasses import dataclass

@dataclass
class SchematicNode:
    """Maps GOSSIP actors to buildable components"""
    component_type: str  # wall, foundation, roof, etc.
    dimensions: Dict[str, float]
    safety_constraints: List[str]
    gossip_actor: str  # Original GOSSIP component
    
    def to_plantuml(self) -> str:
        """Convert to PlantUML diagram notation"""
        return f"rectangle \"{self.component_type}\" as {self.gossip_actor} {{\n" \
               f"  dimensions: {self.dimensions}\n" \
               f"  safety: {self.safety_constraints}\n" \
               f"}}"
