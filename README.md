# A simple TOML Parser 
This repository is made only to convert TOML specific Group and Keys to respectively convert them to Bash Keys.

## Basic Key-Value Pairs
The foundation of TOML is simple assignment.


```toml
name = "TOML Project"
enabled = true
version = 1.0

max_connections = 5000
timeout = 30.5
```

```bash
export NAME="TOML Project"
export ENABLED=true
export VERSION=1.0

export MAX_CONNECTIONS=5000
export TIMEOUT=30.5
```

## Table & Inline Tables
Tables are used to group keys. You define a table with a header in square brackets.

```toml
[database]
server = "192.168.1.1"
ports = [8000, 8001, 8002]
connection_max = 5000
enabled = true

[servers.alpha]
ip = "10.0.0.1"
role = "frontend"

[Servers]
  [Servers.beta]
  port = 6660
  ip = "192.168.0.101"

  [Servers.Admin]
  Name = "Johan"
  Age = 21
```

```bash
export DATABASE_SERVER="192.168.1.1"
export DATABASE_PORTS=(8000 8001 8002)
export DATABASE_CONNECTION_MAX=5000
export DATABASE_ENABLED=true

export SERVERS_ALPHA_IP="10.0.0.1"
export SERVERS_ALPHA_ROLE="frontend"

export SERVERS_BETA_PORT=66600
export SERVERS_BETA_IP="192.168.0.101"
export SERVERS_ADMIN_NAME="Johan"
export SERVERS_ADMIN_AGE=21
```

As we support Tables in TOML. For smaller data structures, you can define everything on one line to save spaces.:

```toml

# Inline Table and infinite scope
geometry = { 
  shape = "circle", 
  properties = { 
    radius = 5, 
    color = "red" 
  } 
}

# Inline Table inside a Parent Group
[Role]
owner = [
  { name = "Developer", email = "dev@example.com" },
  { name = "Developer1", email = "dev@example.com" }
]

products = [
  { name = "Hammer", sku = 738594937 },
  { name = "Nail", sku = 284758393 }
]
```

```bash
declare -A GEOMETRY
GEOMETRY["shape"]="circle"
GEOMETRY["properties_radius"]=5
GEOMETRY["properties_color"]="red"

declare -A ROLE_OWNER
ROLE_OWNER["0_name"]="Developer"
ROLE_OWNER["0_email"]="dev@example.com"
ROLE_OWNER["1_name"]="Developer1"
ROLE_OWNER["1_email"]="dev@example.com"

# Separte from its own Parent due to newline.
declare -A PRODUCTS
PRODUCTS["0_name"]="Hammer"
PRODUCTS["0_sku"]=738594937
PRODUCTS["1_name"]="Nail"
PRODUCTS["1_sku"]=284758393
```

## Comments
In normal standard TOML, the syntax '#' hash is only possible. However, since this is a transpiler. We also allow '//' and '*/'

```TOML

# This is a comment.
// This is a comment. Not valid in the TOML syntax highlighter.
/* Same with this, but valid for the transpiler. */
```


## Indentation and Newline 
This TOML compiler ignores indentation and newline automaticallly. However, when inserting keys. The indentation does matter or else a key may get omitted from its Parent Group.

```toml
[network]
interface = "eth0"
timeout = 30

# Two newlines here reset the scope to root!
# These keys now fall into the global namespace instead of [network]

log_level = "verbose"
retry_limit = 5

[storage]
path = "/mnt/data"
```

```bash
export NETWORK_INTERFACE="eth0"
export NETWORK_TIMEOUT=30

# Indentation and newline has made Keys separte from the Parent
export LOG_LEVEL="verbose"
export RETRY_LIMIT=5

export STORAGE_PATH="/mnt/data"

```
## 
A bit of note.. While this is all great. I do want to point out that this transpiler will spit out config.sh. No support for TOML query and write-read yet.
