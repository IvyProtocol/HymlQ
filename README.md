# CompTomlQ - A TOML-Parser
https://github.com/user-attachments/assets/86d194ff-5329-42ca-9179-d25f6a17a8c8
#
Made in C++26 and is built on top of [CTRE](https://github.com/hanickadot/compile-time-regular-expressions) (Compile-Time Regular Expression), this is a fast [compiler](https://en.wikipedia.org/wiki/Compiler)/[transpiler](https://en.wikipedia.org/wiki/Source-to-source_compiler) that emits [Bash](https://github.com/gitGNU/gnu_bash) Keys by Parsing [TOML](https://toml.io/en/) keys step by step into phases like Lexicon Analysis, Parsing and walking the Abstract Syntax Tree to the [Bash](https://github.com/gitGNU/gnu_bash)-emitter. And thus produces [TOML](https://toml.io/en/) equivalent data transpiled to Bash for the user.

* The Project is built within the Lanugage Framework
* It has been made in the standard way by following: Lexicon Analysis, Parsing, Walking Abstract Syntax Tree to the Transpiler.
* This is a language of its own and is considered a dialect of TOML that doesn't adherent strict behavior.
* Built on top of C++, this Transpiler provides highly faster conversion and zero abstraction granting O(1) speed. [It may vary based on the system and configuration]

> [!NOTE]
> Currently, it is only possible to support Table, Inline Table and Nested Arrays. However, it may become very difficult to support Arrays of Table as they can't
> transpiled directly to Bash and requires key manipulation to store the value. **It is recommended to use Standard Table instead of nested ones as they provide**
> **as they provide clarity while the transpiler emits bash-keys.**

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
